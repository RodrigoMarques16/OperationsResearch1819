#pragma once

#ifndef __solver_h__
#define __solver_h__

#include "helper.h"
#include "task.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <vector>
#include <optional>
#include <algorithm>

#define DEBUG(X) std::cout << #X << ": " << X << '\n';

namespace mad {

/**
 * @brief Structure returned by the solver
 *
 * See mad::Solver
 */
struct [[nodiscard]] Solution {
    const int min_duration;
    const int min_workers;
    const int min_workers_fixed;
    const int critical_workers;
    const std::vector<int> earliest_start;
    const std::vector<int> latest_start;
    const std::vector<int> earliest_finish;
    const std::vector<int> latest_finish;
    const std::vector<int> free_slack;
    const std::vector<int> total_slack;
    const std::vector<int> critical_tasks;
    const std::vector<int> start_times;
    const std::vector<Task> tasks;
};

struct Solver {
    /**
     * @brief Types of events
     */
    enum class EventType {
        CONSUME, /**< Takes workers from available pool   */
        FREE     /**< Frees workers to the available pool */
    };

    /**
     * @brief Events describe the start or finish of a task
     */
    struct Event {
        EventType type; /**< Type of event                        */
        int id;         /**< Id of the task this event represents */
        int tick;       /**< When the event happens               */
        int workers;    /**< How many workers are involved        */

        Event(const EventType& type,
              const int& id,
              const int& tick,
              const int& workers)
            : type(type), id(id), tick(tick), workers(workers) {}

        Event(const Event& other) = default;

        friend bool operator<(const Event& lhs, const Event& rhs) {
            return lhs.tick < rhs.tick;
        }
    };

    /**
     * @brief Construct a new Solver object
     *
     * @param tasks
     */
    explicit Solver(std::vector<Task>& tasks);

    /**
     * @brief Schedule the given tasks
     *
     * @return Solution
     */
    const Solution solve();

private:

    std::vector<Task> tasks;      /**> The list of tasks for this instance */
    std::vector<int> start_tasks; /**> Tasks with no dependencies */
    std::vector<int> final_tasks; /**> Tasks with no dependants */

    std::vector<int> critical_tasks; /**> Tasks which can't be delayed */

    std::vector<int> earliest_start;
    std::vector<int> latest_start;
    std::vector<int> earliest_finish;
    std::vector<int> latest_finish;
    std::vector<int> free_slack;
    std::vector<int> total_slack;
    std::vector<int> start_times;

    int min_duration;
    int min_workers;
    int min_workers_fixed;
    int critical_workers;

    /**
     * @brief Calculate entry and out degrees of each task.
     *
     * Iterate through the task list and calculate entry and out degrees for
     * each task. Stores the values in the member variables in_degree and
     * out_degree
     */
    void degrees();

    /**
     * @brief Determine start and final tasks
     *
     * Start tasks have entry degree == 0
     * Final tasks have out degree == 0
     */
    void key_tasks();

    /**
     * @brief Calculate earliest_start and earliest_finish and the minimum
     * duration of the project
     */
    void forward_pass();

    /**
     * @brief Generate events from tasks
     *
     * A vector of events is generated from this solver's tasks, based on their
     * earliest start and duration.
     *
     * Ordered in chronological order.
     */
    std::vector<Event> make_events(const std::vector<int>& start_time);

    /**
     * @brief Calculate the minimum amount of workers
     * assuming tasks start on their earliest start
     */
    void calc_workers();

    /**
     * @brief Get the transpose of the task graph
     * @return std::vector<Task> the transpose
     */
    std::vector<Task> transpose();

    /**
     * @brief Calculate the latest start and latest finish
     */
    void backward_pass();

    /**
     * @brief Calculate total and free slack
     */
    void calc_slack();

    std::vector<int> calc_free_slack(std::vector<int>);

    /**
     * @brief Finds which tasks are critical and cannot be delayed
     */
    void find_critical();

    /**
     * @brief Calculates the minimum number of workers needed for the project
     * and the start times of each time that achieve that number.
     * @return int
     */
    void calc_minimum_workers();

    /**
     * @brief Simulate an event chain with a fixed number of available workers
     *
     * @param available workers
     * @return true if project can be completed
     * @return false if more workers are needed
     */
    std::optional<std::vector<int>> try_workers(const int available,
                                                const std::vector<int>& start_time,
                                                const std::vector<int>& slack,
                                                const int position = 0);
};

namespace out {

    /**
     * @brief Number of columns to print
     */
    static const int N_COLUMNS = 9;

    static const int COLUMN_WIDTH = 6;

    static const int LINE_WIDTH = (COLUMN_WIDTH + 4) * N_COLUMNS + 2;

    static const int IDENTIFIER_WIDTH = (COLUMN_WIDTH + 4) * 3;

    static const int SMALL_WIDTH = (COLUMN_WIDTH + 4) * 4 + 2;

    /**
     * @brief Name of the columns to print
     */
    static const std::array<std::string, N_COLUMNS> columns = {
        "id", "st", "wrkrs", "es", "ef", "ls", "lf", "ts", "fs"};

    /**
     * @brief Print a formatted solution to stdout
     *
     * @param sol a solution
     */
    void print(const Solution& sol);

    /**
     * @brief Write a solution in csv format to a given file
     *
     * @param sol a solution
     * @param filename the file to write to
     */
    void write_csv(const Solution& sol, std::string& filename);

    template<typename T>
    void print_stat(std::string_view s, T stat, int w);

} // namespace out

} // namespace mad

#endif