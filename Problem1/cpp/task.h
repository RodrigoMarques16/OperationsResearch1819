#pragma once

#ifndef __task_h__
#define __task_h__

#include <iostream>
#include <vector>

namespace mad {

struct Task {
    int id;
    int duration;
    int workers;
    std::vector<int> successors;

    int in_degree = 0;
    int out_degree = 0;

    Task() = default;
    ~Task() = default;

    // Constructor
    Task(const int& id, const int& duration, const int& workers,
         const std::vector<int>& successors)
        : id(id),
          duration(duration),
          workers(workers),
          successors(successors) {}

   Task(const Task& other) = default;

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

}  // namespace mad

#endif