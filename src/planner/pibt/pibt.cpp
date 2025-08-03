#include <planner/pibt/pibt.hpp>

#include <environment/heuristic_matrix.hpp>
#include <utils/assert.hpp>

#include <numeric>

std::vector<std::array<uint32_t, PIBT_PLAN_DEPTH>> get_paths(Robot robot, DirectionType desired) {
    std::array<uint32_t, PIBT_PLAN_DEPTH> nodes_path{}, poses_path{}, edges_path{};
#ifdef ENABLE_ROTATE_MODEL
    uint32_t i = 0;
    if (desired != DirectionType::NONE) {
        for (; static_cast<DirectionType>(robot.pos.get_dir()) != desired; i++) {
            ASSERT(i < 2, "invalid i");
            if (static_cast<DirectionType>((robot.pos.get_dir() + 1) % 4) == desired) {
                edges_path[i] = get_graph().get_to_edge(robot.node, static_cast<uint32_t>(ActionType::ROTATE));
                nodes_path[i] = get_graph().get_to_node(robot.node, static_cast<uint32_t>(ActionType::ROTATE));
                robot.pos = get_graph().get_pos(nodes_path[i]);
                poses_path[i] = robot.pos.get_pos();
                robot.node = nodes_path[i];
            } else {
                edges_path[i] = get_graph().get_to_edge(robot.node, static_cast<uint32_t>(ActionType::COUNTER_ROTATE));
                nodes_path[i] = get_graph().get_to_node(robot.node, static_cast<uint32_t>(ActionType::COUNTER_ROTATE));
                robot.pos = get_graph().get_pos(nodes_path[i]);
                poses_path[i] = robot.pos.get_pos();
                robot.node = nodes_path[i];
            }
        }

        ASSERT(i <= 2, "invalid i");
        edges_path[i] = get_graph().get_to_edge(robot.node, static_cast<uint32_t>(ActionType::FORWARD));
        nodes_path[i] = get_graph().get_to_node(robot.node, static_cast<uint32_t>(ActionType::FORWARD));
        robot.node = nodes_path[i];
        if (!robot.node) {
            robot.pos = Position();
        } else {
            robot.pos = get_graph().get_pos(nodes_path[i]);
            poses_path[i] = robot.pos.get_pos();
            i++;
        }
    }
    for (; i < nodes_path.size(); i++) {
        nodes_path[i] = robot.node;
        poses_path[i] = robot.pos.get_pos();
    }
#else
    if (desired != DirectionType::NONE) {
        nodes_path[0] = get_graph().get_to_node(robot.node, static_cast<uint32_t>(desired));
        edges_path[0] = get_graph().get_to_edge(robot.node, static_cast<uint32_t>(desired));
        if (nodes_path[0]) {
            poses_path[0] = get_graph().get_pos(nodes_path[0]).get_pos();
        }
    } else {
        nodes_path[0] = robot.node;
        poses_path[0] = robot.pos.get_pos();
    }
#endif
    return {nodes_path, poses_path, edges_path};
}

std::array<uint32_t, PIBT_PLAN_DEPTH> PIBT::get_nodes_path(uint32_t r, DirectionType desired) const {
    return get_paths(robots[r], desired)[0];
}

std::array<uint32_t, PIBT_PLAN_DEPTH> PIBT::get_poses_path(uint32_t r, DirectionType desired) const {
    return get_paths(robots[r], desired)[1];
}

std::array<uint32_t, PIBT_PLAN_DEPTH> PIBT::get_edges_path(uint32_t r, DirectionType desired) const {
    return get_paths(robots[r], desired)[2];
}

void PIBT::add_path(uint32_t r) {
    ASSERT(0 <= r && r < robots.size(), "invalid r");

    const auto &poses_path = get_poses_path(r, desires[r]);
    for (uint32_t depth = 0; depth < poses_path.size(); depth++) {
        uint32_t to_pos = poses_path[depth];
        ASSERT(to_pos && to_pos < used_pos.size(), "invalid to_pos");
        ASSERT(used_pos[to_pos][depth] == -1, "already used");
        used_pos[to_pos][depth] = r;
    }

    const auto &edges_path = get_edges_path(r, desires[r]);
    for (uint32_t depth = 0; depth < edges_path.size(); depth++) {
        uint32_t to_edge = edges_path[depth];
        if (to_edge) {
            ASSERT(to_edge < used_edge.size(), "invalid to_edge");
            ASSERT(used_edge[to_edge][depth] == -1, "already used");
            used_edge[to_edge][depth] = r;
        }
    }
}

void PIBT::remove_path(uint32_t r) {
    ASSERT(0 <= r && r < robots.size(), "invalid r");

    auto poses_path = get_poses_path(r, desires[r]);
    for (uint32_t depth = 0; depth < poses_path.size(); depth++) {
        uint32_t to_pos = poses_path[depth];
        ASSERT(to_pos && to_pos < used_pos.size(), "invalid to_pos");
        ASSERT(used_pos[to_pos][depth] == r, "invalid node");
        used_pos[to_pos][depth] = -1;
    }
    auto edges_path = get_edges_path(r, desires[r]);
    for (uint32_t depth = 0; depth < edges_path.size(); depth++) {
        uint32_t to_edge = edges_path[depth];
        if (to_edge) {
            ASSERT(to_edge < used_edge.size(), "invalid to_edge");
            ASSERT(used_edge[to_edge][depth] == r, "invalid edge");
            used_edge[to_edge][depth] = -1;
        }
    }
}

uint32_t PIBT::get_used(uint32_t r, DirectionType desired) const {
    uint32_t answer = -1;

    auto poses_path = get_poses_path(r, desired);
    for (uint32_t depth = 0; depth < poses_path.size(); depth++) {
        uint32_t to_pos = poses_path[depth];
        if (used_pos[to_pos][depth] != -1) {
            if (answer == -1) {
                answer = used_pos[to_pos][depth];
            } else if (answer != used_pos[to_pos][depth]) {
                return -2;
            }
        }
    }

    auto edges_path = get_edges_path(r, desired);
    for (uint32_t depth = 0; depth < edges_path.size(); depth++) {
        uint32_t to_edge = edges_path[depth];
        if (used_edge[to_edge][depth] != -1) {
            if (answer == -1) {
                answer = used_edge[to_edge][depth];
            } else if (answer != used_edge[to_edge][depth]) {
                return -2;
            }
        }
    }
    return answer;
}

PIBT::RetType PIBT::build(uint32_t r, uint32_t priority) {
    if (get_now() >= end_time) {
        return RetType::REJECTED;
    }

    visited[r]++;
    curr_visited[r] = 1;
    DirectionType old_desired = desires[r];

    std::vector<DirectionType> robot_desires;
    for (DirectionType dir: {DirectionType::EAST, DirectionType::SOUTH, DirectionType::WEST, DirectionType::NORTH , DirectionType::NONE}) {
        if (get_nodes_path(r, dir).back()) {
            robot_desires.push_back(dir);
        }
    }

    std::sort(robot_desires.begin(), robot_desires.end(), [&](DirectionType lhs, DirectionType rhs) {
        return get_hm().get(get_nodes_path(r, lhs).back(), robots[r].target) < get_hm().get(get_nodes_path(r, rhs).back(), robots[r].target);
    });

    for (DirectionType desired: robot_desires) {
        desires[r] = desired;
        uint32_t to_r = get_used(r, desired);
        if (to_r == -1) {
            add_path(r);
            return RetType::ACCEPTED;
        } else if (to_r != -2) {
            ASSERT(0 <= to_r && to_r < robots.size(), "invalid to_r: " + std::to_string(to_r));

            if (curr_visited[to_r] ||
                robots[to_r].priority <= priority
#ifdef ENABLE_PIBT_REVISIT
                || visited[to_r] >= 10
#else
                || visited[to_r] > 0
#endif
            ) {
                continue;
            }

            remove_path(to_r);
            add_path(r);

            RetType res = build(to_r, priority);
            if (res == RetType::ACCEPTED) {
                return res;
            } else if (res == RetType::REJECTED) {
                remove_path(r);
                add_path(to_r);
                desires[r] = old_desired;
                return res;
            }

            remove_path(r);
            add_path(to_r);
        }
    }

    desires[r] = old_desired;
    curr_visited[r] = 0;
    return RetType::FAILED;
}

PIBT::PIBT(Robots &robots, TimePoint end_time) : robots(robots), end_time(end_time), desires(robots.size(), DirectionType::NONE), visited(robots.size()), curr_visited(robots.size()) {

    {
        order.resize(robots.size());
        std::iota(order.begin(), order.end(), 0);
        std::stable_sort(order.begin(), order.end(), [&](uint32_t lhs, uint32_t rhs) {
            return robots[lhs].priority < robots[rhs].priority;
        });
    }

    {
        std::array<uint32_t, PIBT_PLAN_DEPTH> value{};
        for (uint32_t depth = 0; depth < value.size(); depth++) {
            value[depth] = -1;
        }
        used_pos.resize(get_map().get_size(), value);
        used_edge.resize(get_graph().get_edges_size(), value);
    }

    for (uint32_t r = 0; r < robots.size(); r++) {
        add_path(r);
    }
}

void PIBT::solve() {
    for (uint32_t r: order) {
        if (get_now() >= end_time) {
            break;
        }
        if (visited[r] <
#ifdef ENABLE_PIBT_REVISIT
            10
#else
            1
#endif
        ) {
            remove_path(r);
            if (build(r, robots[r].priority) != RetType::ACCEPTED) {
                add_path(r);
            }
        }
    }
}

[[nodiscard]] std::vector<DirectionType> PIBT::get_desires() const {
    return desires;
}

[[nodiscard]] std::vector<ActionType> PIBT::get_actions() const {
    std::vector<ActionType> actions(desires.size());
    for (uint32_t r = 0; r < robots.size(); r++) {
#ifdef ENABLE_ROTATE_MODEL
        if (desires[r] == DirectionType::NONE) {
            actions[r] = ActionType::WAIT;
        } else if (static_cast<DirectionType>(robots[r].pos.get_dir()) == desires[r]) {
            actions[r] = ActionType::FORWARD;
        } else if (static_cast<DirectionType>((robots[r].pos.get_dir() + 1) % 4) == desires[r]) {
            actions[r] = ActionType::ROTATE;
        } else {
            actions[r] = ActionType::COUNTER_ROTATE;
        }
#else
        actions[r] = static_cast<ActionType>(desires[r]);
#endif
    }
    return actions;
}
