#include "solver.h"

namespace mad {

Solver::Solver(std::vector<Task>& _tasks) {
    tasks = _tasks;
    earliest_start = std::vector<int>(tasks.size(), 0);
    earliest_finish = std::vector<int>(tasks.size(), 0);
    predecessor = std::vector<int>(tasks.size(), 0);
}

void Solver::solve() { 
    degrees();
    key_tasks();
    forward_pass(); 
    calc_workers();
    backward_pass();
    calc_slack();
    find_critical();
}

void Solver::degrees() {
    std::cout << "Calculating (entry, out) degrees\n";
    for (auto& task : tasks) {
        for (auto& succ_id : task.successors) {
            task.out_degree++;
            tasks[succ_id].in_degree++;
        }
    }

    for (auto& task : tasks)
        std::cout << '\t' << task.id << ": (" << task.in_degree << ", "
                  << task.out_degree << ")\n";

    print_separator();
}

void Solver::key_tasks() {
    std::cout << "Finding key activities\n";
    
    for (auto& task : tasks) {
        if (task.in_degree == 0)
            start_tasks.emplace_back(task.id);
        if(task.out_degree == 0)
            final_tasks.emplace_back(task.id);
    }

    std::cout << "Start tasks:\n\t";
    for(auto& id : start_tasks)
        std::cout << id << ", ";
    std::cout << '\n';

    std::cout << "Final tasks:\n\t";
    for(auto& id : final_tasks)
        std::cout << id << ", ";
    std::cout << '\n';

    print_separator();
}

void Solver::forward_pass() {
    std::cout << "Calculating (ES, EF)\n";

    min_duration = -1;

    std::queue<Task> q;
    
    for(const int& id : start_tasks) {
        q.push(tasks[id]);
        earliest_finish[id] = tasks[id].duration;
        predecessor[id] = -1;
    }

    while (!q.empty()) {

        Task current = q.front();
        q.pop();

        int duration = earliest_start[current.id]+current.duration;
        if (min_duration < duration) {
            min_duration = duration;
        }

        min_duration = std::max(min_duration, earliest_start[current.id]+current.duration);

        for (int& succ_id : current.successors) {
            Task& succ = tasks[succ_id];

            int pred_finish = earliest_start[current.id] + current.duration;
            if (earliest_start[succ_id] < pred_finish) {
                earliest_start[succ_id] = pred_finish;
                earliest_finish[succ_id] = pred_finish + tasks[succ_id].duration;
                predecessor[succ_id] = current.id;
            }

            succ.in_degree--;
            if (succ.in_degree == 0) 
                q.push(succ);
        }
    }

    for (size_t i = 0; i < tasks.size(); i++)
        std::cout << '\t' << i << ": (" << earliest_start[i] << ", "
                  << earliest_finish[i] << ")\n";

    std::cout << "Minimum duration: " << min_duration << '\n';

    print_separator();
}

void Solver::calc_workers() {
    std::cout << "Calculating minimum workers for ES\n";

    std::vector<Event> events;
    
    for(size_t i = 0; i < tasks.size(); i++) {
        events.push_back({
            EventType::CONSUME,
            earliest_start[i],
            tasks[i].workers
        });
        events.push_back({
            EventType::FREE,
            earliest_finish[i],
            tasks[i].workers
        });
    }

    auto compare = [](const Event& lhs, const Event& rhs) {
        return lhs.tick < rhs.tick;
    };

    std::sort(events.begin(), events.end(), compare);

    min_workers = 0;
    int avail_workers = 0;

    for(auto& event : events) {
        if (event.tick != 0) break;
        avail_workers += event.workers;
    } 
    min_workers = avail_workers;

    for(auto& event : events) {
        if (event.type == EventType::FREE) {
            avail_workers += event.workers;
        } else {
            if (avail_workers < event.workers) {
                min_workers += event.workers - avail_workers;
                avail_workers = 0;
            } else {
                avail_workers -= event.workers;
            }
        }
    }

    std::cout << '\t' << min_workers << '\n';

    print_separator();
}

std::vector<Task> Solver::transpose() {
    std::vector<Task> tp = tasks; 

    for(auto& task : tp)
        task.successors.clear();

    for(auto& task : tasks) 
        for(auto& id : task.successors)
            tp[id].successors.emplace_back(task.id);

    std::cout << "Transpose:\n";
    for(auto& task : tp) {
        std::cout << '\t' << task.id << ": ";
        for(auto& id : task.successors)
            std::cout << id << ", ";
        std::cout << '\n';
    }
    print_separator();

    return tp;
}

void Solver::backward_pass() {
    std::vector tp = transpose();
    
    std::cout << "Calculating (LS, LF)\n";
    
    latest_finish = std::vector<int>(tasks.size(), min_duration);
    latest_start = std::vector<int>(tasks.size(), -1);

    std::queue<Task> q;

    for(const int& id : final_tasks) {
        q.push(tp[id]);
        latest_finish[id] = min_duration;
    }

    while(!q.empty()) {
        Task current = q.front();
        q.pop();

        for (const int& id : current.successors) {
            Task& succ = tp[id];

            latest_finish[id] =
                std::min(latest_finish[id],
                         latest_finish[current.id] - current.duration);

            succ.out_degree--;
            if (succ.out_degree == 0) 
                q.push(succ);
        }
    }

    for (size_t i = 0; i < tp.size(); i++) {
        latest_start[i] = latest_finish[i] - tp[i].duration;
        std::cout << '\t' << i << ": (" << latest_start[i] << ", "
                  << latest_finish[i] << ")\n"; 
    }
    print_separator();
}

void Solver::calc_slack() {
    total_slack = std::vector<int>(tasks.size());
    free_slack = std::vector<int>(tasks.size());

    std::cout << "Calculating (total, free) slack\n";
    for(const Task& task : tasks) {
        int id = task.id;
        
        total_slack[id] = latest_start[id] - earliest_start[id];

        int min_es = min_duration;
        for(const int& succ : task.successors)
            min_es = std::min(min_es, earliest_start[succ]);

        free_slack[id] = min_es - earliest_finish[id];

        std::cout << '\t' << id << ": (" << total_slack[id] << ", " << free_slack[id] << ")\n";
    }
    print_separator();
}

void Solver::find_critical() {
    std::cout << "Calculating critical tasks\n\t";
    critical_workers = 0;
    
    for(const Task& task : tasks) {
        if (total_slack[task.id] == 0) {
            critical_tasks.emplace_back(task.id);
            critical_workers += task.workers;
            
            std::cout << task.id << ", ";
        }
    }

    std::cout << "\n\tCritical workers: " << critical_workers << '\n';
    print_separator();
}

template<typename A>
void print_queue(std::queue<A> q, int iter) {
    std::cout << "Queue on iteration " << iter << '\n';
    while(!q.empty()) {
        std::cout << q.front().id << " ";
        q.pop();
    }
    std::cout << '\n';
}

}  // namespace mad

