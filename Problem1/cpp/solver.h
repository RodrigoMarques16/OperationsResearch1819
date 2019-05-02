#pragma once

#ifndef __solver_h__
#define __solver_h__

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include "task.h"

namespace mad {

struct Solver {

    std::vector<Task> tasks;
    std::vector<Task&> start_tasks;
    
    std::vector<int> earliest_start;
    std::vector<int> predecessor;

    explicit Solver(std::vector<Task>& tasks);

    void degrees();
    void find_start();
    void calc_es();
    void solve();
    void sim();
    void task_sim(Task& task);
};

}  // namespace mad

#endif