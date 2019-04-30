#include <iostream>
#include "parser.cpp"
#include "task.h"

using namespace ::mad;

void tarefa(int id, int ids[], int dur, int ntrab);

int main(int argc, char** argv) {
    Parser parser(argv[1]);
    std::vector<Task> tasks = parser.parse();

    std::cout << "Data read:"
              << "\n\n";
    for (auto& task : tasks) task.print();
    std::cout << "==================\n";

    return 0;
}