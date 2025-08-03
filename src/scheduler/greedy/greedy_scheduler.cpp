#include <scheduler/greedy/greedy_scheduler.hpp>

#include <environment/heuristic_matrix.hpp>
#include <environment/info.hpp>
#include <environment/robot.hpp>
#include <environment/task.hpp>
#include <utils/assert.hpp>
#include <utils/time.hpp>
#include <utils/tools.hpp>

#include <algorithm>
#include <atomic>
#include <queue>
#include <set>
#include <unordered_set>

void GreedyScheduler::rebuild_dp(uint32_t r) {
    dp[r].clear();
    for (uint32_t t: free_tasks) {
        dp[r].emplace_back(get_dist(r, t), t);
    }
    std::sort(dp[r].begin(), dp[r].end());
    timestep_updated[r] = timestep + 1;
}

void GreedyScheduler::rebuild_dp(TimePoint end_time) {
    std::vector<uint32_t> order = free_robots;
    std::stable_sort(order.begin(), order.end(), [&](uint32_t lhs, uint32_t rhs) {
        return timestep_updated[lhs] < timestep_updated[rhs];
    });

    std::atomic<uint32_t> counter{};
    for (uint32_t i = 0; i < order.size() && get_now() < end_time; i++) {
        rebuild_dp(order[i]);
        ++counter;
    }
    /*launch_threads(THREADS_NUM_VALUE, [&](uint32_t thr) {
        for (uint32_t i = thr; i < order.size() && get_now() < end_time; i += THREADS_NUM_VALUE) {
            rebuild_dp(order[i]);
            ++counter;
        }
    });*/
}

uint64_t GreedyScheduler::get_dist(uint32_t r, uint32_t t) const {
    if (t == -1) {
        return 1e6;
    }

    uint32_t source = robots[r].node;
    uint64_t dist_to_target = get_hm().get(source, task_target[t]);
    uint64_t dist = dist_to_target * 5 + task_metric[t];
    ASSERT(static_cast<uint32_t>(dist) == dist, "overflow");
    return dist;
}

void GreedyScheduler::remove(uint32_t r) {
    ASSERT(0 <= r && r < desires.size(), "invalid r");
    uint32_t t = desires[r];
    if (t == -1) {
        return;
    }

    cur_score -= get_dist(r, t);
    task_to_robot[t] = -1;
    desires[r] = -1;
}

void GreedyScheduler::add(uint32_t r, uint32_t t) {
    ASSERT(0 <= r && r < desires.size(), "invalid r");
    ASSERT(0 <= t && t < task_to_robot.size(), "invalid t");
    ASSERT(desires[r] == -1, "already have task");
    ASSERT(task_to_robot[t] == -1, "already have robot");

    cur_score += get_dist(r, t);
    task_to_robot[t] = r;
    desires[r] = t;
}

void GreedyScheduler::validate() {
    std::set<uint32_t> S;
    for (uint32_t r = 0; r < desires.size(); r++) {
        if (desires[r] != -1) {
            ASSERT(!S.count(desires[r]), "already contains");
            S.insert(desires[r]);
            ASSERT(task_to_robot[desires[r]] == r, "invalid task to robot");
        }
    }
}

GreedyScheduler::GreedyScheduler(Robots &robots, TaskPool &task_pool) : robots(robots), task_pool(task_pool) {
}

void GreedyScheduler::update(uint32_t timestep) {
    this->timestep = timestep;
    desires.resize(robots.size(), -1);
    timestep_updated.resize(desires.size());
    dp.resize(desires.size());

    free_robots.clear();
    free_tasks.clear();

    // build free_tasks
    for (auto &[t, task]: task_pool) {
        if (
                task.agent_assigned == -1// нет агента
                || !task.is_taken        // мы можем поменять задачу
        ) {
            if (task.agent_assigned != -1) {
                robots[task.agent_assigned].task_id = -1;
                robots[task.agent_assigned].target = 0;
                task.agent_assigned = -1;
            }
            free_tasks.push_back(t);
        }
    }

    // build free_robots
    for (uint32_t r = 0; r < robots.size(); r++) {
        uint32_t t = robots[r].task_id;

        // есть задача и она в процессе выполнения
        // не можем ее убрать
        if (task_pool.contains(t) && task_pool.at(t).is_taken) {
            desires[r] = t;
            continue;
        }
        // нет задачи
        if (!task_pool.contains(t) || !task_pool.at(t).is_taken) {
            free_robots.push_back(r);
        }
    }

    // build task_metric, task_target
    {
        Timer timer;
        for (uint32_t t: free_tasks) {
            if (task_metric.size() <= t) {
                task_metric.resize(t + 1, -1);
                task_target.resize(t + 1);
            }
            auto &task = task_pool.at(t);
            task_target[t] = task.targets[0];

            uint32_t d = 0;
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
            task_metric[t] = d;
        }
    }

    for (uint32_t t: free_tasks) {
        if (t >= task_to_robot.size()) {
            task_to_robot.resize(t + 1, -1);
        }
        task_to_robot[t] = -1;
    }

    cur_score = 0;
    for (uint32_t r: free_robots) {
        desires[r] = -1;
        cur_score += get_dist(r, desires[r]);
    }
    validate();
}

void GreedyScheduler::solve(TimePoint end_time) {
    Timer timer;
    for (uint32_t r: free_robots) {
        remove(r);
    }
    std::unordered_set<uint32_t> used_task;

    auto validate_task = [&](uint32_t task_id) {
        // task is already used
        if (used_task.count(task_id)) {
            return false;
        }
        // this task is not available
        if (!task_pool.contains(task_id)) {
            return false;
        }
        // robot already used this task
        if (task_pool.at(task_id).agent_assigned != -1) {
            return false;
        }
        return true;
    };

    // (dist, r, index)
    std::priority_queue<std::tuple<uint32_t, uint32_t, uint32_t>, std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>, std::greater<>> Heap;
    for (uint32_t r: free_robots) {
        if (!dp[r].empty()) {
            Heap.push({dp[r][0].first, r, 0});
        }
    }

    while (!Heap.empty() && get_now() < end_time) {
        auto [dist, r, index] = Heap.top();
        Heap.pop();

        uint32_t task_id = dp[r][index].second;
        ASSERT(dist == dp[r][index].first, "invalid dist");

        if (!validate_task(task_id)) {
            index++;

            if (index < dp[r].size()) {
                Heap.push({dp[r][index].first, r, index});
            }

            continue;
        }

        ASSERT(task_pool.contains(task_id), "no contains");
        ASSERT(task_pool.at(task_id).agent_assigned == -1, "already assigned");
        ASSERT(!used_task.count(task_id), "already used");

        add(r, task_id);
        used_task.insert(task_id);
    }

    validate();

    // если робот без задачи, то мы ее дадим. это нужно для WPPL, который не может без цели, он не самурай
    auto it = free_tasks.begin();
    for (uint32_t r = 0; r < desires.size(); r++) {
        if (desires[r] == -1) {
            while (it != free_tasks.end() && task_to_robot[*it] != -1) {
                it++;
            }
            ASSERT(it != free_tasks.end(), "unable to set task");
            add(r, *it);
        }
    }
}

std::vector<uint32_t> GreedyScheduler::get_schedule() const {
    return desires;
}

double GreedyScheduler::get_score() const {
    return cur_score;
}
