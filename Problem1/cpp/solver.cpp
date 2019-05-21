#include "solver.h"


namespace mad {

using Event = Solver::Event;

Solver::Solver(std::vector<Task>& _tasks) {
    tasks           = _tasks;
    size_t size     = tasks.size();
    earliest_start  = std::vector<int>(size, 0);
    latest_start    = std::vector<int>(size, 0);
    earliest_finish = std::vector<int>(size, 0);
    latest_finish   = std::vector<int>(size, 0);
    free_slack      = std::vector<int>(size, 0);
    total_slack     = std::vector<int>(size, 0);
    start_times     = std::vector<int>(size, 0);
}

const Solution Solver::solve() {
    degrees();
    key_tasks();
    forward_pass();
    backward_pass();
    calc_slack();
    find_critical();
    calc_workers();
    calc_minimum_workers();

    return {
        min_duration,
        min_workers,
        min_workers_fixed,
        critical_workers,
        earliest_start,
        latest_start,
        earliest_finish,
        latest_finish,
        free_slack,
        total_slack,
        critical_tasks,
        start_times,
        tasks
    };
}

void Solver::degrees() {
    for (Task& task : tasks) {
        task.out_degree = task.successors.size();
        for (const int& succ_id : task.successors)
            tasks[succ_id].in_degree++;
    }
}

void Solver::key_tasks() {
    for (const Task& task : tasks) {
        if (task.in_degree == 0)
            start_tasks.emplace_back(task.id);
        if (task.out_degree == 0)
            final_tasks.emplace_back(task.id);
    }
}

std::vector<Event> Solver::make_events(const std::vector<int>& start_time) {
    std::vector<Event> events;

    for (size_t i = 0; i < tasks.size(); i++) {
        events.emplace_back(EventType::CONSUME, tasks[i].id, start_time[i],
                            tasks[i].workers);
        events.emplace_back(EventType::FREE, tasks[i].id,
                            start_time[i] + tasks[i].duration,
                            tasks[i].workers);
    }

    std::sort(events.begin(), events.end());

    return events;
}

void Solver::forward_pass() {
    std::queue<Task> q;

    min_duration = -1;
    
    for (const int& id : start_tasks) {
        q.push(tasks[id]);
        earliest_finish[id] = tasks[id].duration;
    }

    while (!q.empty()) {
        const Task current = q.front();
        q.pop();

        const int duration = earliest_start[current.id] + current.duration;

        min_duration = std::max(min_duration, duration);

        for (const int& succ_id : current.successors) {
            Task& succ = tasks[succ_id];

            int pred_finish = earliest_start[current.id] + current.duration;

            if (earliest_start[succ_id] < pred_finish) {
                earliest_start[succ_id]  = pred_finish;
                earliest_finish[succ_id] = pred_finish + tasks[succ_id].duration;
            }

            succ.in_degree--;
            if (succ.in_degree == 0)
                q.push(succ);
        }
    }
}

void Solver::calc_workers() {
    int available = 0;
    critical_workers = 0;

    const auto& events = make_events(earliest_start);

    auto event_it = events.begin();
    while(event_it->tick == 0) {
        available += event_it->workers;
        if (tasks[event_it->id].is_critical)
            critical_workers += event_it->workers;
        event_it++;
    }

    min_workers_fixed = available;
    
    //std::cout << available << '\n';
    for (const auto& event : events) {
        //std::cout << "Tick " << event.tick << '\n';
        //DEBUG(available);

        if (event.type == EventType::FREE) {
            //std::cout << "Task " << event.id+1 << " frees "
            //          << event.workers << " workers\n";
            available += event.workers;
        } else {
            if (available < event.workers) {
                //std::cout << "Task " << event.id+1 << " consumes "
                //          << available << " and requests "
                //          << event.workers - available << " workers\n";
                min_workers_fixed += event.workers - available;
                if (tasks[event.id].is_critical)
                    critical_workers += event.workers - available;
                available = 0;
            } else {
                //std::cout << "Task " << event.id+1 << " consumes "
                //          << event.workers << " workers\n";
                available -= event.workers;
            }
        }

        //std::cout << "===\n";
    }
}

std::vector<Task> Solver::transpose() {
    std::vector<Task> tp = tasks;

    for (Task& task : tp)
       task.successors.clear();

    for (const Task& task : tasks)
       for (const int& id : task.successors)
          tp[id].successors.emplace_back(task.id);
    
    return tp;
}

void Solver::backward_pass() {
    std::vector tp = transpose();

    latest_finish = std::vector<int>(tasks.size(), min_duration);
    latest_start  = std::vector<int>(tasks.size(), -1);

    std::queue<Task> q;

    for (const int& id : final_tasks)
        q.push(tp[id]);

    while (!q.empty()) {
        const Task current = q.front();
        q.pop();

        for (const int& id : current.successors) {
            Task& succ = tp[id];

            int new_ls = latest_finish[current.id] - current.duration;
            if (latest_finish[id] > new_ls) {
                latest_finish[id] = new_ls;
                latest_start[id] = new_ls - tp[id].duration;
            }

            succ.out_degree--;
            if (succ.out_degree == 0)
                q.push(succ);
        }
    }

    for (size_t i = 0; i < tp.size(); i++)
        latest_start[i] = latest_finish[i] - tp[i].duration;
}

void Solver::calc_slack() {
    total_slack = std::vector<int>(tasks.size());
    free_slack  = std::vector<int>(tasks.size());

    for (const Task& task : tasks) {
        const int id = task.id;

        total_slack[id] = latest_start[id] - earliest_start[id];

        int min_es = min_duration;
        for (const int& succ : task.successors)
            min_es = std::min(min_es, earliest_start[succ]);

        free_slack[id] = min_es - earliest_finish[id];
    }
}

void Solver::find_critical() {
    for (Task& task : tasks) {
        if (total_slack[task.id] == 0) {
            critical_tasks.emplace_back(task.id);
            task.is_critical = true;
        }
    }
}

std::optional<std::vector<int>>
Solver::try_workers(const int workers,
                    const std::vector<int>& start_time,
                    const std::vector<int>& slack,
                    const int position) {
    int available = workers;
    std::vector<Event> events = make_events(start_time);

    for(size_t i = position; i < events.size(); i++) {
        const Event& event = events[i];

        if (event.type == EventType::FREE) {
            available += event.workers;
        } else {
            if (event.workers > available) {
                if (slack[event.id] == 0)
                    return std::nullopt;

                auto new_start_time = std::vector<int>(start_time);
                auto new_slack      = std::vector<int>(slack);
                new_start_time[event.id]++;
                new_slack[event.id]--;

                return try_workers(workers, new_start_time, new_slack);
            } else {
                available -= event.workers;
            }
        }
    }

    return start_time;
}


void Solver::calc_minimum_workers() {
    int low  = critical_workers;
    int high = min_workers_fixed;

    start_times = earliest_start;

    while (low < high) {
        int mid = low + ((high - low) / 2);
        auto st = try_workers(mid, earliest_start, free_slack);
        if (st.has_value()){
            high = mid;
            start_times = *st;
        }
        else {
            low = mid + 1;
        }
    }

    min_workers = low;
}

namespace out {

    template<typename T>
    void print_stat(std::string s, T stat) {
        std::cout << "|| " << s 
                  << std::setw(IDENTIFIER_WIDTH - s.length()) << "|| "
                  << std::setw(COLUMN_WIDTH) << stat << " ||\n";
        print_separator(SMALL_WIDTH);
    }

    void print(const Solution& sol) {
        print_separator(LINE_WIDTH);

        std::cout << "|| ";
        for (int i = 0; i < N_COLUMNS; i++)
            std::cout << std::setw(COLUMN_WIDTH) << columns[i] << " || ";
        std::cout << '\n';
        print_separator(LINE_WIDTH);

        for (size_t i = 0; i < sol.tasks.size(); i++) {
            if(sol.tasks[i].is_critical)
                std::cout << "|| *" << std::setw(COLUMN_WIDTH-1) << sol.tasks[i].id + 1 << " || ";
            else
                std::cout << "|| " << std::setw(COLUMN_WIDTH) << sol.tasks[i].id + 1 << " || ";
            
            std::cout << std::right 
                      << std::setw(COLUMN_WIDTH) << sol.start_times[i]     << " || "
                      << std::setw(COLUMN_WIDTH) << sol.tasks[i].workers   << " || "
                      << std::setw(COLUMN_WIDTH) << sol.earliest_start[i]  << " || "
                      << std::setw(COLUMN_WIDTH) << sol.earliest_finish[i] << " || "
                      << std::setw(COLUMN_WIDTH) << sol.latest_start[i]    << " || "
                      << std::setw(COLUMN_WIDTH) << sol.latest_finish[i]   << " || "
                      << std::setw(COLUMN_WIDTH) << sol.total_slack[i]     << " || "
                      << std::setw(COLUMN_WIDTH) << sol.free_slack[i]      << " ||\n";
        }

        print_separator(LINE_WIDTH);

        print_stat("Minimum duration", sol.min_duration);
        print_stat("Minimum workers (fixed)", sol.min_workers_fixed);
        print_stat("Minimum workers", sol.min_workers);
        print_stat("Critical workers", sol.critical_workers);
    }

} // namespace out

} // namespace mad
