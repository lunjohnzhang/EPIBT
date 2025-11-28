#include <environment/graph.hpp>
#include <environment/graph_guidance.hpp>
#include <environment/guidance_map.hpp>
#include <environment/heuristic_matrix.hpp>
#include <environment/info.hpp>
#include <environment/map.hpp>
#include <environment/robot.hpp>
#include <environment/task.hpp>
#include <environment/test_system.hpp>

#include <environment/environment.hpp>
#include <planner/causal_pibt/heuristics.hpp>

#include <planner/epibt/operations.hpp>
#include <planner/epibt/operations_map.hpp>

#include <utils/assert.hpp>
#include <utils/config.hpp>
#include <utils/tools.hpp>

#include <filesystem>
#include <iostream>
#include <mutex>
#include <numeric>

int main(int argc, char *argv[]) {
    Timer main_timer;
    RuntimeConfig config;

    std::cout << "Threads num: " << THREADS_NUM_VALUE << std::endl;

    if (argc >= 2) {
        std::string config_file = argv[1];
        std::cout << "Loading config file: " << config_file << std::endl;
        config = load_config(config_file);
    } else {
        std::cout << "No config file specified" << std::endl;
        return 0;
    }
    apply_runtime_config(config);

    std::ifstream(config.map_file) >> get_map();

#ifndef ENABLE_ROTATE_MODEL
    ASSERT(get_planner_type() != PlannerType::CAUSAL_PIBT && get_planner_type() != PlannerType::PIBT_TF,
           "incomplete planner type for non rotate model");
#endif

    get_gg() = GraphGuidance(get_map().get_rows(), get_map().get_cols());
    if (config.graph_guidance_type == GraphGuidanceType::DISABLE) {

    } else if (config.graph_guidance_type == GraphGuidanceType::ENABLE) {
        if (get_map_type() == MapType::RANDOM) {
            get_guidance_map() = GuidanceMap(get_map_type(), get_map());
            get_gg() = GraphGuidance(get_guidance_map());
        } else if (get_map_type() == MapType::CITY) {
            get_gg().set_city();
        } else if (get_map_type() == MapType::GAME) {
            get_gg().set_game();
        } else if (get_map_type() == MapType::SORTATION) {
            get_gg().set_sortation();
        } else if (get_map_type() == MapType::WAREHOUSE) {
            get_gg().set_warehouse();
        } else {
            FAILED_ASSERT("unexpected map type");
        }
        ASSERT(get_planner_type() != PlannerType::CAUSAL_PIBT && get_planner_type() != PlannerType::PIBT_TF, "Graph Guidance can't be used with Causal PIBT or Causal PIBT+traffic flow");
    } else {
        FAILED_ASSERT("unexpected graph guidance type");
    }

    get_graph() = Graph(get_map(), get_gg());
    get_hm() = HeuristicMatrix(get_graph());

    if (get_planner_type() == PlannerType::EPIBT || get_planner_type() == PlannerType::EPIBT_LNS || get_planner_type() == PlannerType::PEPIBT_LNS) {
        init_operations();
        get_omap() = OperationsMap(get_graph(), get_operations());
    }

#ifdef ENABLE_ROTATE_MODEL
    Environment env;
    {
        env.num_of_agents = 0;
        env.rows = get_map().get_rows();
        env.cols = get_map().get_cols();
        env.map.assign(get_map().get_size() - 1, -1);
        for (uint32_t pos = 1; pos < get_map().get_size(); pos++) {
            env.map[pos - 1] = !get_map().is_free(pos);
        }
    }
    if (get_planner_type() == PlannerType::PIBT_TF || get_planner_type() == PlannerType::CAUSAL_PIBT) {
        CausalPlanner::init_heuristics(&env);
        launch_threads(THREADS_NUM_VALUE, [&](uint32_t thr) {
            for (uint32_t dst = thr; dst + 1 < get_map().get_size(); dst += THREADS_NUM_VALUE) {
                for (uint32_t src = 0; src + 1 < get_map().get_size(); src++) {
                    if (get_map().is_free(src + 1) && get_map().is_free(dst + 1)) {
                        CausalPlanner::get_h(&env, src, dst);
                    }
                }
            }
        });
    }
#endif

    std::filesystem::create_directories(config.output_path);

    std::mutex mutex;


    if (config.n_tasks > 0) {
        Robots robots(config.n_robots);
        robots.initialize_start_positions(get_map(), config.seed);
        TaskPool task_pool;
        task_pool.gen_random_tasks(config.n_tasks, get_map(), config.seed);

        TestSystem test_system(robots, task_pool);

        Answer answer = test_system.simulate(config.steps_num);

        answer.write_result_to_json(config.output_path + "result.json");

    } else {
        std::vector<bool> visited;
        for (uint32_t i = 0;; i++) {
            std::string filename = config.agents_path + "/agents_" + std::to_string(i) + ".csv";
            if (!std::filesystem::exists(filename)) {
                break;
            }
            visited.push_back(false);
        }
        if (visited.empty()) {
            visited.push_back(false);
        }

        std::cout << "The preparation is over: " << main_timer << std::endl;

        launch_threads(THREADS_NUM_VALUE, [&](uint32_t thr) {
            for (uint32_t test = 0; test < visited.size(); test++) {
                {
                    std::unique_lock locker(mutex);
                    if (visited[test]) {
                        continue;
                    }
                    visited[test] = true;
                    std::cout << "Start test " << test << ", thr: " << thr << std::endl;
                }

                Timer timer;

                Robots robots;
                if (config.agents_path.size() >= 4 && config.agents_path.substr(config.agents_path.size() - 4) == ".csv") {
                    std::ifstream(config.agents_path) >> robots;
                } else {
                    std::ifstream(config.agents_path + "/agents_" + std::to_string(test) + ".csv") >> robots;
                }

                TaskPool task_pool;
                if (config.tasks_path.size() >= 4 && config.tasks_path.substr(config.tasks_path.size() - 4) == ".csv") {
                    std::ifstream(config.tasks_path) >> task_pool;
                } else {
                    std::ifstream(config.tasks_path + "/tasks_" + std::to_string(test) + ".csv") >> task_pool;
                }

                TestSystem test_system(robots, task_pool);

                std::string test_dir = config.output_path + std::to_string(test) + "/";
                std::filesystem::create_directories(test_dir);

                Answer answer = test_system.simulate(config.steps_num);

                answer.write_log(std::ofstream(test_dir + "log.csv"));

                for (uint32_t action = 0; action <= ACTIONS_NUM; action++) {
                    std::string filename = test_dir + "heatmap_";
                    if (action < ACTIONS_NUM) {
                        filename += action_to_char(static_cast<ActionType>(action));
                    } else {
                        filename += "all";
                    }
                    filename += ".csv";
                    answer.write_heatmap(std::ofstream(filename), action);
                }

                {
                    std::ofstream output(test_dir + "metrics.csv");
                    output << "metric,value\n";
                    output << "task type,";
#ifdef ENABLE_ROTATE_MODEL
                    output << "LMAPF-T\n";
#else
                output << "LMAPF\n";
#endif
                    output << "map type," << map_type_to_string(config.map_type) << '\n';
                    output << "test id," << test << '\n';
                    output << "scheduler type," << scheduler_type_to_string(config.scheduler_type) << '\n';
                    output << "planner type," << planner_type_to_string(config.planner_type);
                    if (config.planner_type == PlannerType::EPIBT || config.planner_type == PlannerType::EPIBT_LNS || config.planner_type == PlannerType::PEPIBT_LNS) {
                        output << "(" << EPIBT_DEPTH_VALUE << ")";
                    }
                    output << '\n';
                    output << "graph guidance type,";
                    if (config.graph_guidance_type == GraphGuidanceType::ENABLE) {
                        output << "enable\n";
                    } else {
                        output << "disable\n";
                    }
                    output << "agents num," << robots.size() << '\n';
                    output << "steps num," << config.steps_num << '\n';
                    output << "finished tasks," << answer.finished_tasks.size() << '\n';
                    output << "throughput," << static_cast<double>(answer.finished_tasks.size()) / config.steps_num << '\n';
                    for (uint32_t action = 0; action < ACTIONS_NUM; action++) {
                        output << action_to_char(static_cast<ActionType>(action)) << "," << static_cast<double>(answer.actions_num.back()[action]) * 100 / config.steps_num / robots.size() << "\n";
                    }
                    output << "avg step time (ms)," << std::accumulate(answer.step_time.begin(), answer.step_time.end(), 0.0) * 1000 / config.steps_num << '\n';
                    output << "avg scheduler time (ms)," << std::accumulate(answer.scheduler_time.begin(), answer.scheduler_time.end(), 0.0) * 1000 / config.steps_num << '\n';
                    output << "avg planner time (ms)," << std::accumulate(answer.planner_time.begin(), answer.planner_time.end(), 0.0) * 1000 / config.steps_num << '\n';

                    {
                        std::unique_lock locker(mutex);
                        std::cout << "Done  test " << test << ", thr: " << thr << ", time: " << timer << std::endl;
                    }
                }
            }
        });
    }


    std::cout << "===Done===" << std::endl;
    std::cout << "Total time: " << main_timer << std::endl;
}
