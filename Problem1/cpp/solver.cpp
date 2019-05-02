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
    for (auto& task : tasks)
        for (auto& succ_id : task.successors) {
            task.out_degree++;
            tasks[succ_id].in_degree++;
        }
}

void Solver::find_start() {
    for (auto& task : tasks)
        if (task.in_degree == 0) start_tasks.push_back(task);
}

void Solver::solve() { calc_es(); }

int clamp_min(int value, int min) {
    if (value < min) return min;
    return value;
}

struct compare {
    bool cmp(const Task& lhs, const Task& rhs) {
        return lhs.duration < rhs.duration;
    }
};

void Solver::calc_es() {
    int min_duration = -1;
    int min_workers = 0;
    int avail_workers = 0;

    auto compare = [](const Task& lhs, const Task& rhs) {
        return lhs.id < rhs.id;
    };

    std::priority_queue<Task&, std::vector<Task&>, decltype(compare)> q(
        compare, start_tasks);

    while (!q.empty()) {
        Task& current = q.top();
        q.pop();

        min_duration = std::max(min_duration, earliest_start[current.id]);

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

}  // namespace mad

std::atomic<int> tick, workers, workers_available;
void Solver::task_sim(Task& task) {
    int time_left = task.duration;
    while (time_left--) tick++;
    workers_available += task.workers;
    for (int next : task.successors) task_sim(tasks[next]);
}

void Solver::sim() {
    tick = workers = workers_available = 0;

    for (Task& task : start_tasks) {
        workers_available += task.workers;
        std::thread t([&]() {
            int time_left = task.duration;
            while (time_left--) {
                tick++;
            }
        });
    }
}

}  // namespace mad