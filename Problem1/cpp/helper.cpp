#include "helper.h"

int clamp_min(int value, int min) {
    if (value < min) return min;
    return value;
}

void print_separator(int x) {
    while(x--)
        std::cout << "=";
    std::cout << '\n';
}