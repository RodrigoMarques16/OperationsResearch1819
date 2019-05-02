#include "solver.h"

namespace mad {

Solver::Solver(std::vector<Task>& _tasks) {
    tasks = _tasks;
    earliest_start = std::vector<int>(tasks.size(), 0);
    predecessor = std::vector<int>(tasks.size(), 0);

    degrees();
    find_start();
}

void Solver::degrees() {
    std::cout << "Calculating entry degrees\n";
    for (auto& task : tasks) {
        for (auto& succ_id : task.successors) {
            task.out_degree++;
            tasks[succ_id].in_degree++;
        }
    }

    for (auto& task : tasks)
        std::cout << '\t' << task.id << ": " << task.in_degree << '\n';

    print_separator();
}

void Solver::find_start() {
    std::cout << "Finding start activities\n";
    for (auto& task : tasks){
        if (task.in_degree == 0) {
            start_tasks.emplace_back(task.id);
            std::cout << '\t' << task.id << '\n';
        }
    }

    print_separator();
}

void Solver::solve() { 
    calc_es(); 
}

template<typename A, typename B, typename C>
void print_queue(std::priority_queue<A,B,C> q, int iter) {
    std::cout << "Queue on iteration " << iter << '\n';
    while(!q.empty()) {
        std::cout << q.top().id << " ";
        q.pop();
    }
    std::cout << '\n';
}

void Solver::calc_es() {
    std::cout << "Calculating (ES, EF)\n";

    //int tick = 0;
    int min_duration = -1;
    //int min_workers = 0;
    //int avail_workers = 0;
    int iter = 1;

    auto compare = [](const Task& lhs, const Task& rhs) {
        return lhs.duration < rhs.duration;
    };

    // switch to struct {int duration, int id} if Task is too slow
    std::priority_queue<Task, std::vector<Task>, decltype(compare)> q(compare);
    
    for(const int& id : start_tasks) {
        q.push(tasks[id]);
        //avail_workers += tasks[id].workers;
    }

    while (!q.empty()) {
        //print_queue(q, iter);
        iter++;

        Task current = q.top();
        q.pop();

        min_duration = std::max(min_duration, earliest_start[current.id]+current.duration);

        for (int& succ_id : current.successors) {
            Task& succ = tasks[succ_id];

            earliest_start[succ_id] =
                std::max(earliest_start[succ_id],
                         earliest_start[current.id] + current.duration);

            succ.in_degree--;
            if (succ.in_degree == 0) 
                q.push(succ);
        }
    }

    for(size_t i = 0; i < tasks.size(); i++)
        std::cout << '\t' << i << ": (" << earliest_start[i] << ", " << earliest_start[i] + tasks[i].duration << ")\n";
    std::cout << "Minimum duration: " << min_duration << '\n';
    print_separator();
}

}  // namespace mad
