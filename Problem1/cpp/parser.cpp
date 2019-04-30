#include "parser.h"
#include <iostream>

namespace mad {

const std::vector<Task> Parser::parse() const {
    std::ifstream task_file(filename);
    std::vector<Task> tasks;

    if (!task_file.is_open()) {
        std::cout << "Failed to open file " << filename << '\n';
        throw;
    } else {
        std::string s;

        // Read file line by line
        while (std::getline(task_file, s)) {
            // Build a stringstream from the line read
            std::stringstream ss{s};
            int id, duration, workers, n_sucessors = 0;
            std::vector<int> successors;

            // Read data
            ss >> id >> n_sucessors;

            successors.resize(n_sucessors);

            for (int i = 0; i < n_sucessors; ++i) ss >> successors[i];

            ss >> duration >> workers;

            // Create a new task from data read
            tasks.emplace_back(id, duration, workers, successors);
        }
    }

    return tasks;
}
}  // namespace mad