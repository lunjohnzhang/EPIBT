#include <environment/answer.hpp>

#include <environment/graph.hpp>
#include <utils/assert.hpp>

#include <fstream>
#include <iomanip>
#include <numeric>
#include <unordered_map>

#include <nlohmann/json.hpp>

void Answer::validate_actions(uint32_t step) const {
    std::unordered_map<uint32_t, uint32_t> pos_usage, edge_usage;
    for (uint32_t r = 0; r < robots.size(); r++) {
        Position pos = robots[r].positions[step];
        ActionType action = robots[r].actions[step];
        Position to = pos.simulate(action);

        {
            uint32_t pos_id = to.get_pos();
            ASSERT(!pos_usage.contains(pos_id), "vertex collision");
            pos_usage[pos_id] = r;
        }

        if (
#ifdef ENABLE_ROTATE_MODEL
                action == ActionType::FORWARD
#else
                action != ActionType::WAIT
#endif
        ) {
            uint32_t edge_id = get_graph().get_to_edge(get_graph().get_node(pos), static_cast<uint32_t>(action));
            ASSERT(!edge_usage.contains(edge_id), "edge collision");
            edge_usage[edge_id] = r;
        }
    }
}

void Answer::validate_schedule(uint32_t step) const {
    // task_used[task_id] = assigned agent
    std::unordered_map<uint32_t, uint32_t> task_used;
    for (uint32_t r = 0; r < robots.size(); r++) {
        uint32_t task_id = robots[r].assigned_task[step];
        if (task_id != -1) {
            ASSERT(tasks[step].contains(task_id), "invalid task id");
            ASSERT(tasks[step].at(task_id).task_id == task_id, "invalid task id");

            if (tasks[step].at(task_id).is_taken) {
                ASSERT(tasks[step].at(task_id).agent_assigned == r, "invalid task assigned");
            }
            ASSERT(!task_used.contains(task_id), "task collision");
            task_used[task_id] = r;
        }
    }
}

void Answer::write_heatmap(std::ostream &output, uint32_t action) const {
    output << "row\\col";
    for (uint32_t col = 0; col < get_map().get_cols(); col++) {
        output << ',' << col;
    }
    output << '\n';
    for (uint32_t row = 0; row < get_map().get_rows(); row++) {
        output << row;
        for (uint32_t col = 0; col < get_map().get_cols(); col++) {
            output << ',';
            if (get_map().is_free(row * get_map().get_cols() + col + 1)) {
                output << heatmap[row][col][action];
            } else {
                output << -1;
            }
        }
        output << '\n';
    }
}

void Answer::write_heatmap(std::ostream &&output, uint32_t action) const {
    write_heatmap(output, action);
}

void Answer::write_log(std::ostream &output) const {
    output << "timestep,finished tasks,";
    for (uint32_t action = 0; action < ACTIONS_NUM; action++) {
        output << action_to_char(static_cast<ActionType>(action)) << ',';
    }
    output << "time(ms),scheduler time(ms),planner time(ms)\n";

    for (uint32_t timestep = 0; timestep < steps_num; timestep++) {
        output << timestep << ',' << finished_tasks_in_step[timestep] << ',';
        for (uint32_t action = 0; action < ACTIONS_NUM; action++) {
            output << actions_num[timestep][action] << ',';
        }
        output << static_cast<uint32_t>(step_time[timestep] * 1000) << ',' << static_cast<uint32_t>(scheduler_time[timestep] * 1000) << ',' << static_cast<uint32_t>(planner_time[timestep] * 1000) << '\n';
    }

    output << std::fixed << std::setprecision(1);
    output << "total," << finished_tasks.size() << ',';
    for (uint32_t action = 0; action < ACTIONS_NUM; action++) {
        output << (static_cast<double>(actions_num.back()[action]) * 100 / steps_num) / static_cast<double>(robots.size()) << "%,";
    }
    output << std::accumulate(step_time.begin(), step_time.end(), 0.0) * 1000 << ',' << std::accumulate(scheduler_time.begin(), scheduler_time.end(), 0.0) * 1000 << ',' << std::accumulate(planner_time.begin(), planner_time.end(), 0.0) * 1000 << '\n';
}

void Answer::write_log(std::ostream &&output) const {
    write_log(output);
}

void Answer::write_agent() const {
    uint32_t r = 0;
    for (auto action: robots[r].actions) {
        std::cout << action_to_char(action);
    }
}

void Answer::write_result_to_json(const std::string &filename) const {
    std::ofstream file(filename);
    nlohmann::json result;
    double throughput = static_cast<double>(this->finished_tasks.size()) / this->steps_num;
    double cpu_runtime = std::accumulate(this->step_time.begin(), this->step_time.end(), 0.0);
    std::cout << "Throughput: " << throughput << ", CPU runtime: " << cpu_runtime << "s" << std::endl;
    // Flatten the heatmap and add to JSON
    std::vector<uint64_t> vertex_wait_matrix;
    std::vector<uint64_t> move_usage_matrix;
    std::vector<uint64_t> cr_usage_matrix;
    std::vector<uint64_t> ccr_usage_matrix;
    for (const auto &row: this->heatmap) {
        for (const auto &usage: row) {
            // WAIT action is at the last index
            vertex_wait_matrix.push_back(usage[static_cast<uint32_t>(ActionType::WAIT)]);
#ifdef ENABLE_ROTATE_MODEL
            // With rotation model, only FORWARD is moving
            move_usage_matrix.push_back(usage[static_cast<uint32_t>(ActionType::FORWARD)]);
            cr_usage_matrix.push_back(usage[static_cast<uint32_t>(ActionType::ROTATE)]);
            ccr_usage_matrix.push_back(usage[static_cast<uint32_t>(ActionType::COUNTER_ROTATE)]);
#else
            // Without rotation model, sum all directional moves
            move_usage_matrix.push_back(
                    usage[static_cast<uint32_t>(ActionType::EAST)] +
                    usage[static_cast<uint32_t>(ActionType::SOUTH)] +
                    usage[static_cast<uint32_t>(ActionType::WEST)] +
                    usage[static_cast<uint32_t>(ActionType::NORTH)]);
#endif
        }
    }

    result = {
            {"success", true},
            {"throughput", throughput},
            {"cpu_runtime", cpu_runtime},
            {"vertex_wait_matrix", vertex_wait_matrix},
            {"move_usage_matrix", move_usage_matrix},
#ifdef ENABLE_ROTATE_MODEL
            {"cr_usage_matrix", cr_usage_matrix},
            {"ccr_usage_matrix", ccr_usage_matrix},
#endif
    };

    file << std::setw(4) << result << std::endl;
}