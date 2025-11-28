#pragma once

#include <cstdint>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

#include <environment/map.hpp>
#include <utils/randomizer.hpp>

struct Task {
    uint32_t task_id = -1;
    uint32_t agent_assigned = -1;
    std::vector<uint32_t> targets;
    bool is_taken = false;
};

class TaskPool {
    std::unordered_map<uint32_t, Task> pool;

    // for generate next task
    // [i] = sequence of targets for some task
    std::vector<std::vector<uint32_t>> task_targets;
    uint32_t next_task = 0;

    // [r] = number of finished tasks
    std::vector<uint32_t> task_counter;

public:
    [[nodiscard]] bool contains(uint32_t task_id) const;

    Task &at(uint32_t task_id);

    const Task &at(uint32_t task_id) const;

    void insert(const Task &task);

    void erase(uint32_t task_id);

    [[nodiscard]] uint32_t size() const;

    std::unordered_map<uint32_t, Task>::iterator begin();

    std::unordered_map<uint32_t, Task>::iterator end();

    std::unordered_map<uint32_t, Task>::const_iterator begin() const;

    std::unordered_map<uint32_t, Task>::const_iterator end() const;

    void gen_next_task();

    uint32_t gen_const_next_task(uint32_t r, uint32_t agents_num);

    friend std::istream &operator>>(std::istream &input, TaskPool &pool);

    void gen_random_tasks(int n_tasks, const Map &map, int seed);
};
