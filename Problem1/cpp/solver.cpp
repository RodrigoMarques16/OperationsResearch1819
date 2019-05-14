#include "solver.h"

namespace mad {

Solver::Solver(std::vector<Task>& _tasks) {
    tasks           = _tasks;
    size_t size     = tasks.size();
    earliest_start  = std::vector<int>(size, 0);
    latest_start    = std::vector<int>(size, 0);
    earliest_finish = std::vector<int>(size, 0);
    latest_finish   = std::vector<int>(size, 0);
    free_slack      = std::vector<int>(size, 0);
    total_slack     = std::vector<int>(size, 0);
}

const Solution Solver::solve() {
    degrees();
    key_tasks();
    forward_pass();
    make_events();
    calc_workers();
    backward_pass();
    calc_slack();
    find_critical();
    //minimum_workers();

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
        tasks
    };
}

void Solver::degrees() {
    for (auto& task : tasks) {
        task.out_degree = task.successors.size();
        for (auto& succ_id : task.successors)
            tasks[succ_id].in_degree++;
    }
}

void Solver::key_tasks() {
    for (auto& task : tasks) {
        if (task.in_degree == 0)
            start_tasks.emplace_back(task.id);
        if (task.out_degree == 0)
            final_tasks.emplace_back(task.id);
    }
}

void Solver::make_events() {
    for (size_t i = 0; i < tasks.size(); i++) {
        events.emplace_back(EventType::CONSUME, tasks[i].id, earliest_start[i],
                            tasks[i].workers);
        events.emplace_back(EventType::FREE, tasks[i].id, earliest_finish[i],
                            tasks[i].workers);
    }

    std::sort(events.begin(), events.end());
}

void Solver::forward_pass() {
    std::queue<Task> q;

    min_duration = -1;
    
    for (const int& id : start_tasks) {
        q.push(tasks[id]);
        earliest_finish[id] = tasks[id].duration;
    }

    while (!q.empty()) {
        Task current = q.front();
        q.pop();

        int duration = earliest_start[current.id] + current.duration;

        min_duration = std::max(min_duration, duration);

        for (int& succ_id : current.successors) {
            Task& succ = tasks[succ_id];

            int pred_finish = earliest_start[current.id] + current.duration;

            if (earliest_start[succ_id] < pred_finish) {
                earliest_start[succ_id]  = pred_finish;
                earliest_finish[succ_id] = pred_finish
                                           + tasks[succ_id].duration;
            }

            succ.in_degree--;
            if (succ.in_degree == 0)
                q.push(succ);
        }
    }
}

void Solver::calc_workers() {
    int available = 0;

    auto event = events.begin();
    while(event->tick == 0) {
        available += event->workers;
        event++;
    }

    min_workers_fixed = available;
    
    for (auto& event : events) {
        if (event.type == EventType::FREE) {
            available += event.workers;
        } else {
            if (available < event.workers) {
                min_workers_fixed += event.workers - available;
                available = 0;
            } else {
                available -= event.workers;
            }
        }
    }
}

std::vector<Task> Solver::transpose() {
    std::vector<Task> tp = tasks;

    for (auto& task : tp)
       task.successors.clear();

    for (auto& task : tasks)
       for (auto& id : task.successors)
          tp[id].successors.emplace_back(task.id);
          
    /*
    std::vector<Task> tp = std::vector<Task>(tasks.size());
    
    for (auto& task : tasks) {
        tp[task.id] = Task(task.id, task.duration, task.workers, std::vector<int>());
        for (auto& id : task.successors)
            tp[id].successors.emplace_back(task.id);
    }
    */
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
        Task current = q.front();
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
        int id = task.id;

        total_slack[id] = latest_start[id] - earliest_start[id];

        int min_es = min_duration;
        for (const int& succ : task.successors)
            min_es = std::min(min_es, earliest_start[succ]);

        free_slack[id] = min_es - earliest_finish[id];
    }
}

void Solver::find_critical() {
    critical_workers = 0;

    for (const Task& task : tasks) {
        if (total_slack[task.id] == 0) {
            critical_tasks.emplace_back(task.id);
            critical_workers += task.workers;
        }
    }
}

bool Solver::try_workers(int available) {
    std::priority_queue<Event> q(std::less<Event>(), events);
    std::vector<int> slack = total_slack;

    while (!q.empty()) {
        Event event = q.top();
        q.pop();

        if (event.type == EventType::FREE)
            available += event.workers;
        else {
            if (event.workers > available) {
                if (slack[event.id] > 0) {
                    // dfs divergence point here?
                    event.tick += slack[event.id];
                    slack[event.id]--;
                    q.push(event);
                } else {
                    return false;
                }
            } else {
                available -= event.workers;
            }
        }
    }

    return true;
}

void Solver::minimum_workers() {
    int low        = critical_workers;
    int high       = min_workers_fixed;
    int iterations = 0;

    while (low < high) {
        iterations++;
        int mid = low + ((high - low) / 2);
        std::cout << "BS(" << low << ", " << mid << ", " << high << ")\n";

        if (try_workers(mid))
            high = mid;
        else
            low = mid + 1;
    }

    std::cout << "Minimum workers: " << low << '\n';

}

namespace out {
    void print(const Solution& sol) {
        
        std::cout << "\n==============================================================\n"
                  << "|| ";
        for (int i = 0; i < N_COLUMNS; i++)
            std::cout << std::setw(3) << columns[i] << " || ";
        std::cout << '\n'
                  << "==============================================================\n";
        for (size_t i = 0; i < sol.tasks.size(); i++) {
            std::cout << "|| " << std::setw(3) << sol.tasks[i].id << " || "
                      << std::setw(3) << sol.earliest_start[i] << " || "
                      << std::setw(3) << sol.earliest_finish[i] << " || "
                      << std::setw(3) << sol.latest_start[i] << " || "
                      << std::setw(3) << sol.latest_finish[i] << " || "
                      << std::setw(3) << sol.total_slack[i] << " || "
                      << std::setw(3) << sol.free_slack[i] << " || "
                      << std::setw(7) << sol.tasks[i].workers << " ||\n";
        }
        std::cout << "==============================================================\n";
        std::cout << "|| Minimum duration" << std::setw(12) << " || " << sol.min_duration << std::setw(30) << "||\n";
        std::cout << "==============================================================\n";
        std::cout << "|| Minimum workers (fixed) " << " || " << sol.min_workers_fixed << std::setw(31) << "||\n";
        std::cout << "==============================================================\n";
    }
} // namespace out

} // namespace mad
