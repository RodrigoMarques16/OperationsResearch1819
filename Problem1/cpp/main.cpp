#include <iostream>
#include <vector>

#include "parser.h"
#include "task.h"
#include "solver.h"

using namespace ::mad;

void tarefa(int id, int ids[], int dur, int ntrab);

int main(int argc, char** argv) {
    Parser parser(argv[1]);
    std::vector<Task> tasks = parser.parse();

    std::cout << "Data read:"
              << "\n\n";
    for (auto& task : tasks) task.print();
        std::cout << "==================\n";

    Solver solver = Solver(tasks);
    solver.solve();

    return 0;
}