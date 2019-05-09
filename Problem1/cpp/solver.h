#pragma once

#ifndef __solver_h__
#define __solver_h__

#include <iostream>
#include <atomic>
#include <queue>
#include <thread>
#include <vector>
#include <algorithm>

#include "helper.h"
#include "task.h"

namespace mad {

struct Solver {

    enum class EventType { CONSUME, FREE };
    struct Event {
        EventType type;
        int tick;
        int workers;
    };

    std::vector<Task> tasks;
    std::vector<int> start_tasks;
    std::vector<int> final_tasks;
    std::vector<int> critical_tasks;

    std::vector<int> earliest_start;
    std::vector<int> latest_start;
    std::vector<int> earliest_finish;
    std::vector<int> latest_finish;
    std::vector<int> free_slack;
    std::vector<int> total_slack;
    std::vector<int> predecessor;

    int min_duration;
    int min_workers;
    int critical_workers; 

    explicit Solver(std::vector<Task>& tasks);

    void solve();
    void degrees();
    void key_tasks();
    void forward_pass();
    void calc_workers();
    std::vector<Task> transpose();
    void backward_pass();
    void calc_slack();
    void find_critical();

};

}  // namespace mad

#endif