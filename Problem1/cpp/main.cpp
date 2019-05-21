#include <iostream>
#include <vector>

#include "parser.h"
#include "task.h"
#include "solver.h"
#include "helper.h"

using namespace ::mad;
using namespace ::mad::out;

int main(int argc, char** argv) {
    Parser parser(argv[1]);
    std::vector<Task> tasks = parser.parse();

    Solver solver = Solver(tasks);
    Solution sol = solver.solve();

    print(sol);

    return 0;
}