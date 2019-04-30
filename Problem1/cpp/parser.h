#pragma once

#ifndef __parser_h__
#define __parser_h__

//#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "task.h"

namespace mad {

struct Parser {
    std::string filename;

    explicit Parser(std::string filename) : filename(filename) {};

    const std::vector<Task> parse() const;
};

}  // namespace mad

#endif