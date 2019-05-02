#include <iostream>
#include <vector>

#include "parser.h"
#include "task.h"
#include "solver.h"
#include "helper.h"

using namespace ::mad;

int main(int argc, char** argv) {
    Parser parser(argv[1]);
    std::vector<Task> tasks = parser.parse();

    std::cout << "Reading data\n";
    for (auto& task : tasks)
        task.print();
    print_separator();


    Solver solver = Solver(tasks);
    solver.solve();

    return 0;
}