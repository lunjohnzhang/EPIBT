#pragma once

#include <environment/action_model.hpp>
#include <environment/position.hpp>
#include <environment/task.hpp>

#include <map>

struct Answer {
    struct Robot {
        // initial position
        std::vector<Position> positions;
        // [step] = action in this step
        std::vector<ActionType> actions;
        // [step] = assigned task in this step
        std::vector<uint32_t> assigned_task;
    };

    uint32_t steps_num = 0;

    std::vector<Robot> robots;

    // [step] = task pool
    std::vector<std::map<uint32_t, Task>> tasks;

    // [row][col][action]
    std::vector<std::vector<std::array<uint64_t, ACTIONS_NUM + 1>>> heatmap;

    // (timestep, task_id, agent_id)
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> finished_tasks;

    // [step][action]
    std::vector<std::array<uint32_t, ACTIONS_NUM>> actions_num;

    // [step]
    std::vector<uint32_t> finished_tasks_in_step;

    // [step]
    std::vector<double> scheduler_time;

    // [step]
    std::vector<double> planner_time;

    // [step]
    std::vector<double> step_time;

    void validate_actions(uint32_t step) const;

    void validate_schedule(uint32_t step) const;

    void write_heatmap(std::ostream &output, uint32_t action) const;

    void write_heatmap(std::ostream &&output, uint32_t action) const;

    void write_log(std::ostream &output) const;

    void write_log(std::ostream &&output) const;

    void write_agent() const;

    // Write result to a JSON file in RHCR format.
    void write_result_to_json(const std::string &filename) const;
};

