#include "parser.h"
#include <iostream>

namespace mad {

const std::vector<Task> Parser::parse() const {
    std::ifstream task_file(filename);

    if (!task_file.is_open()) {
        std::cout << "Failed to open file " << filename << '\n';
        throw;
    } else {
        std::string s;

        std::getline(task_file, s);
        std::stringstream ss{s};
        
        int num_tasks;
        ss >> num_tasks;

        std::vector<Task> tasks = std::vector<Task>(num_tasks);

        // Read file line by line
        while (std::getline(task_file, s)) {
            // Build a stringstream from the line read
            std::stringstream ss{s};
            int id, duration, workers, n_sucessors = 0;
            std::vector<int> successors;

            // Read data
            ss >> id >> n_sucessors;

            successors.resize(n_sucessors);

            for (int i = 0; i < n_sucessors; ++i) {
                ss >> successors[i];
                successors[i]--;
            }

            ss >> duration >> workers;

            // Create a new task from data read
            tasks[id-1] = Task(id-1, duration, workers, successors);
        }

        return tasks;
    }
}

}  // namespace mad