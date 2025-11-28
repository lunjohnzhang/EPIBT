#include <environment/map.hpp>
#include <environment/task.hpp>
#include <utils/assert.hpp>
#include <utils/tools.hpp>

bool TaskPool::contains(uint32_t task_id) const {
    return pool.contains(task_id);
}

Task &TaskPool::at(uint32_t task_id) {
    ASSERT(pool.contains(task_id), "no contains");
    return pool.at(task_id);
}

const Task &TaskPool::at(uint32_t task_id) const {
    ASSERT(pool.contains(task_id), "no contains");
    return pool.at(task_id);
}

void TaskPool::insert(const Task &task) {
    ASSERT(!pool.contains(task.task_id), "already contains");
    pool.emplace(task.task_id, task);
}

void TaskPool::erase(uint32_t task_id) {
    ASSERT(pool.contains(task_id), "no contains");
    pool.erase(task_id);
}

[[nodiscard]] uint32_t TaskPool::size() const {
    return pool.size();
}

std::unordered_map<uint32_t, Task>::iterator TaskPool::begin() {
    return pool.begin();
}

std::unordered_map<uint32_t, Task>::iterator TaskPool::end() {
    return pool.end();
}

std::unordered_map<uint32_t, Task>::const_iterator TaskPool::begin() const {
    return pool.begin();
}

std::unordered_map<uint32_t, Task>::const_iterator TaskPool::end() const {
    return pool.end();
}

void TaskPool::gen_next_task() {
    insert(Task{next_task, static_cast<uint32_t>(-1), task_targets[next_task % task_targets.size()]});
    next_task++;
}

uint32_t TaskPool::gen_const_next_task(uint32_t r, uint32_t agents_num) {
    if (task_counter.size() <= r) {
        task_counter.resize(agents_num);
    }
    Task task{next_task, static_cast<uint32_t>(-1), task_targets[(task_counter[r] * static_cast<uint64_t>(agents_num) + r) % task_targets.size()]};
    task.targets.resize(1);
    task_counter[r]++;
    next_task++;
    insert(task);
    return task.task_id;
}

std::istream &operator>>(std::istream &input, TaskPool &pool) {
    ASSERT(input, "invalid input");
    std::string line;
    std::getline(input, line);
    ASSERT(line == "targets", "invalid header");
    while (std::getline(input, line)) {
        auto strs = split(line, ',');
        pool.task_targets.emplace_back();
        for (auto &s: strs) {
            uint32_t pos = std::stoll(s) + 1;
            pool.task_targets.back().push_back(pos);
            ASSERT(get_map().is_free(pos), "is not free");
        }
    }
    return input;
}

void TaskPool::gen_random_tasks(int n_tasks, const Map &map, int seed) {
    Randomizer rnd(seed);
    for (int i = 0; i < n_tasks; i++) {
        this->task_targets.emplace_back();
        uint32_t pos = map.sample_free_location(rnd);
        ASSERT(map.is_free(pos), "is not free");
        this->task_targets.back().push_back(pos);
    }
}