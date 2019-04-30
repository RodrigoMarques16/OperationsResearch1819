#include "solver.h"

namespace mad {

Solver::Solver(std::vector<Task>& _tasks) {
    tasks          = _tasks;
    earliest_start = std::vector<int>(tasks.size(), 0);
    predecessor    = std::vector<int>(tasks.size(), 0);

    degrees();
    find_start();
}

void Solver::degrees() {
    for (auto& task : tasks)
        for (auto& succ_id : task.successors) {
            task.out_degree++;
            tasks[succ_id].in_degree++;
        }
}

void Solver::find_start() {
    for (auto& task : tasks)
        if (task.in_degree == 0) 
            start_tasks.push_back(task);
}

void Solver::calc_es() {
    int min_duration = -1;
    std::queue<Task, std::vector<Task>> q(start_tasks);

    while(!q.empty()) { 

    }

}

}  // namespace mad