#pragma once

#ifndef __task_h__
#define __task_h__

#include <iostream>
#include <vector>

namespace mad {

/**
 * @brief Describes a task
 *
 * Structure to describe a task with duration, number of workers and dependant
 * tasks.
 */
struct Task {

    int id;
    int duration;
    int workers;

    int in_degree    = 0;
    int out_degree   = 0;
    bool is_critical = false;

    std::vector<int> successors;

    Task()  = default;
    ~Task() = default;

    /**
     * @brief Default constructor
     *
     * @param id
     * @param duration
     * @param workers
     * @param successors
     */
    Task(const int& id,
         const int& duration,
         const int& workers,
         const std::vector<int>& successors)
        : id(id), duration(duration), workers(workers), successors(successors) {
    }

    /**
     * @brief Copy constructor
     *
     * @param other
     */
    Task(const Task& other) = default;

    /**
     * @brief Print a task
     *
     * Prints a task in a single line
     */
    void print() {
        std::cout << '\t' << id << ": "
                  << "  duration: " << duration << " workers: " << workers
                  << " succeded by: ";
        for (auto& succ : successors)
            std::cout << succ << " ";
        std::cout << "\n";
    }

    friend bool operator<(const Task& lhs, const Task& rhs) {
        return lhs.duration < rhs.duration;
    }
};

} // namespace mad

#endif