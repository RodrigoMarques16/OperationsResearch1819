#pragma once

#ifndef __parser_h__
#define __parser_h__

#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "task.h"

namespace mad {

/**
 * @brief Parses task text files
 * 
 */
class Parser {
    std::string filename;

public:

    /**
     * @brief Construct a new Parser object
     * 
     * @param filename File to open
     */
    explicit Parser(std::string filename) : filename(filename) {};

    /**
     * @brief Parses this parser's file and returns a vector of tasks
     * 
     * @return const std::vector<Task> 
     */
    const std::vector<Task> parse() const;
};

}  // namespace mad

#endif