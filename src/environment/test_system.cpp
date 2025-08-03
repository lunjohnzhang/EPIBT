#include <environment/test_system.hpp>

#include <environment/graph.hpp>
#include <environment/heuristic_matrix.hpp>
#include <environment/info.hpp>
#include <environment/map.hpp>
#include <environment/robot.hpp>
#include <environment/task.hpp>

#include <utils/randomizer.hpp>

#include <planner/pibt/pibt.hpp>

#include <planner/causal_pibt/planner.hpp>

#include <planner/epibt/epibt.hpp>
#include <planner/epibt/epibt_lns.hpp>
#include <planner/epibt/epibt_lns_old.hpp>
#include <planner/epibt/operations.hpp>
#include <planner/epibt/operations_map.hpp>
#include <planner/epibt/pepibt_lns.hpp>

#include <fstream>
#include <map>
#include <set>

void TestSystem::gen_random_agents() {
    Randomizer rnd;
    std::set<uint32_t> agents;
    uint32_t agents_num = 800;
    while (agents.size() < agents_num) {
        uint32_t pos = rnd.get(1, get_map().get_size() - 1);
        if (get_map().is_free(pos) && !agents.contains(pos)) {
            agents.insert(pos);
        }
    }
    {
        std::ofstream output("agents_" + std::to_string(agents_num) + ".csv");
        output << "agent id,row,col\n";
        uint32_t id = 0;
        for (uint32_t pos: agents) {
#ifdef ENABLE_ROTATE_MODEL
            Position p(pos, 0);
#else
            Position p(pos);
#endif
            output << id << ',' << p.get_row() << ',' << p.get_col() << '\n';
            id++;
        }
    }
    std::exit(0);
}

void TestSystem::update(Answer &answer) {
    // update tasks
    for (uint32_t r = 0; r < robots.size(); r++) {
        auto &robot = robots[r];
        if (robot.task_id != -1) {
            auto &task = task_pool.at(robot.task_id);
            if (robot.pos.get_pos() == task.targets[0]) {
                task.is_taken = true;
                task.targets.erase(task.targets.begin());
                if (task.targets.empty()) {
                    robot.task_id = -1;
                    robot.target = 0;
                    robot.priority = -1;
                    answer.finished_tasks.emplace_back(timestep, task.task_id, r);
                    finished_tasks.push_back(task.task_id);
                    task_pool.erase(task.task_id);
                }
            }
        }
    }

    // update metrics
    for (uint32_t r = 0; r < robots.size(); r++) {
        auto &robot = robots[r];
        if (robot.task_id != -1) {
            auto &task = task_pool.at(robot.task_id);
            robot.target = task.targets[0];
            robot.priority = 0;

            uint32_t d = get_hm().get(robot.node, robot.target);
            for (int i = 0; i + 1 < task.targets.size(); i++) {
                uint32_t source = task.targets[i];
                uint32_t target = task.targets[i + 1];
#ifdef ENABLE_ROTATE_MODEL
                Position pos(source, 0);
#else
                Position pos(source);
#endif
                d += get_hm().get(get_graph().get_node(pos), target);
            }
            robot.priority = d;
        }
    }
}

std::vector<uint32_t> TestSystem::get_schedule() {
    std::vector<uint32_t> schedule;
    if (get_scheduler_type() == SchedulerType::CONST) {
        schedule.resize(robots.size());
        for (uint32_t r = 0; r < robots.size(); r++) {
            auto &robot = robots[r];
            if (robot.task_id == -1) {
                while (true) {
                    uint32_t task_id = task_pool.gen_const_next_task(r, robots.size());
                    if (robot.pos.get_pos() == task_pool.at(task_id).targets[0]) {
                        continue;// already here, skip bad task
                    }
                    schedule[r] = task_id;
                    break;
                }
            } else {
                schedule[r] = robot.task_id;
            }
        }
    } else if (get_scheduler_type() == SchedulerType::GREEDY) {
        greedy_scheduler->update(timestep);
        greedy_scheduler->rebuild_dp(get_now() + Milliseconds(500));
        greedy_scheduler->solve(get_now() + Milliseconds(500));
        schedule = greedy_scheduler->get_schedule();
    }
    return schedule;
}

std::vector<ActionType> TestSystem::get_actions() {
    std::vector<ActionType> actions;
    uint32_t PLANNER_TIME_LIMIT = get_step_time();
    TimePoint end_time = get_now() + Milliseconds(PLANNER_TIME_LIMIT);

#ifndef ENABLE_EPIBT_IO
    std::vector<uint32_t> epibt_prev_operations(robots.size());
#endif

    if (get_planner_type() == PlannerType::EPIBT) {
        EPIBT solver(robots, end_time, epibt_prev_operations);
        solver.solve();
        actions = solver.get_actions();
        epibt_prev_operations = solver.get_desires();
        for (uint32_t r = 0; r < epibt_prev_operations.size(); r++) {
            epibt_prev_operations[r] = get_operation_next(epibt_prev_operations[r]);
        }
    } else if (get_planner_type() == PlannerType::EPIBT_LNS) {
        EPIBT_LNS solver(robots, end_time, epibt_prev_operations);
        solver.solve(rnd.get());
        actions = solver.get_actions();
        epibt_prev_operations = solver.get_desires();
        for (uint32_t r = 0; r < epibt_prev_operations.size(); r++) {
            epibt_prev_operations[r] = get_operation_next(epibt_prev_operations[r]);
        }
    }
#ifdef ENABLE_ROTATE_MODEL
    else if (get_planner_type() == PlannerType::EPIBT_LNS_OLD) {
        EPIBT_LNS_OLD solver(robots, end_time);
        solver.solve(rnd.get());
        actions = solver.get_actions();
    }
#endif
    else if (get_planner_type() == PlannerType::PEPIBT_LNS) {
        PEPIBT_LNS solver(robots, end_time, epibt_prev_operations);
        solver.solve(rnd.get());
        actions = solver.get_actions();
        epibt_prev_operations = solver.get_desires();
        for (uint32_t r = 0; r < epibt_prev_operations.size(); r++) {
            epibt_prev_operations[r] = get_operation_next(epibt_prev_operations[r]);
        }
    } else if (get_planner_type() == PlannerType::PIBT) {
        PIBT solver(robots, end_time);
        solver.solve();
        actions = solver.get_actions();
    }
#ifdef ENABLE_ROTATE_MODEL
    else if (get_planner_type() == PlannerType::PIBT_TF || get_planner_type() == PlannerType::CAUSAL_PIBT || get_planner_type() == PlannerType::WPPL) {
        env.goal_locations.assign(robots.size(), {});
        env.curr_states.assign(robots.size(), {});
        for (uint32_t r = 0; r < robots.size(); r++) {
            auto targets = task_pool.at(robots[r].task_id).targets;
            for (uint32_t target: targets) {
                env.goal_locations[r].push_back(target - 1);
            }
            env.curr_states[r].timestep = timestep;
            env.curr_states[r].location = robots[r].pos.get_pos() - 1;
            env.curr_states[r].orientation = robots[r].pos.get_dir();
        }
        env.curr_timestep = timestep;

        if (get_planner_type() == PlannerType::WPPL) {
            wppl_planner->plan(PLANNER_TIME_LIMIT, actions);
        } else {
            causal_pibt_planner->plan(end_time, actions, &env);
        }
    }
#endif
    else {
        FAILED_ASSERT("unexpected planner type: " + planner_type_to_string(get_planner_type()));
    }
    return actions;
}

TestSystem::TestSystem(Robots copy_robots, TaskPool copy_task_pool) : robots(std::move(copy_robots)), task_pool(std::move(copy_task_pool)), epibt_prev_operations(this->robots.size()) {
    // gen_random_agents();

    if (get_scheduler_type() == SchedulerType::GREEDY) {
        greedy_scheduler = std::make_unique<GreedyScheduler>(robots, task_pool);
    }

#ifdef ENABLE_ROTATE_MODEL
    if (get_planner_type() == PlannerType::PIBT_TF || get_planner_type() == PlannerType::CAUSAL_PIBT || get_planner_type() == PlannerType::WPPL) {
        env.num_of_agents = robots.size();
        env.rows = get_map().get_rows();
        env.cols = get_map().get_cols();
        env.map.assign(get_map().get_size() - 1, -1);
        for (uint32_t pos = 1; pos < get_map().get_size(); pos++) {
            env.map[pos - 1] = !get_map().is_free(pos);
        }
    }

    if (get_planner_type() == PlannerType::PIBT_TF || get_planner_type() == PlannerType::CAUSAL_PIBT) {
        causal_pibt_planner = std::make_unique<CausalPIBT>();
        causal_pibt_planner->initialize(&env);
    }

    if (get_planner_type() == PlannerType::WPPL) {
        wppl_planner = std::make_unique<WPPL>();
        wppl_planner->initialize(&env);
    }
#endif
}

Answer TestSystem::simulate(uint32_t steps_num) {
    Answer answer;
    answer.steps_num = steps_num;
    answer.robots.resize(robots.size());
    answer.heatmap = std::vector(get_map().get_rows(), std::vector(get_map().get_cols(), std::array<uint64_t, ACTIONS_NUM + 1>()));

    for (; timestep < steps_num; timestep++) {
        Timer step_timer;

        for (uint32_t r = 0; r < answer.robots.size(); r++) {
            answer.robots[r].positions.push_back(robots[r].pos);
        }

        update(answer);

        // update task pool
        if (get_scheduler_type() != SchedulerType::CONST) {
            while (task_pool.size() * 2 < robots.size() * 3) {
                task_pool.gen_next_task();
            }
        }

        // call scheduler
        std::vector<uint32_t> schedule;
        {
            Timer timer;
            schedule = get_schedule();
            answer.scheduler_time.push_back(timer.get());
        }

        for (uint32_t r = 0; r < robots.size(); r++) {
            answer.robots[r].assigned_task.push_back(schedule[r]);
        }

        answer.tasks.emplace_back();
        for (auto &[id, task]: task_pool) {
            answer.tasks[timestep][id] = task;
        }
        answer.validate_schedule(timestep);

        // set new schedule
        for (uint32_t r = 0; r < robots.size(); r++) {
            uint32_t old_t = robots[r].task_id;
            if (old_t != -1) {
                task_pool.at(old_t).agent_assigned = -1;
            }
            robots[r].task_id = schedule[r];
            task_pool.at(schedule[r]).agent_assigned = r;
        }

        update(answer);

        // call planner
        std::vector<ActionType> actions;
        {
            Timer timer;
            actions = get_actions();
            answer.planner_time.push_back(timer.get());
        }

        for (uint32_t r = 0; r < robots.size(); r++) {
            answer.robots[r].actions.push_back(actions[r]);
        }

        answer.validate_actions(timestep);

        std::array<uint32_t, ACTIONS_NUM> actions_num{};
        for (uint32_t r = 0; r < robots.size(); r++) {
            auto &robot = robots[r];
            auto action = static_cast<uint32_t>(actions[r]);
            answer.heatmap[robot.pos.get_row()][robot.pos.get_col()][action]++;
            answer.heatmap[robot.pos.get_row()][robot.pos.get_col()].back()++;
            actions_num[action]++;

            robot.pos = robot.pos.simulate(actions[r]);
            robot.node = get_graph().get_node(robot.pos);
        }
        answer.actions_num.push_back(actions_num);

        update(answer);

        answer.step_time.push_back(step_timer.get());
        answer.finished_tasks_in_step.push_back(finished_tasks.size());
        finished_tasks.clear();

        answer.tasks.back().clear();// save memory
    }

    answer.actions_num.push_back({});
    for (timestep = 0; timestep < steps_num; timestep++) {
        for (uint32_t action = 0; action < ACTIONS_NUM; action++) {
            answer.actions_num.back()[action] += answer.actions_num[timestep][action];
        }
    }
    return answer;
}
