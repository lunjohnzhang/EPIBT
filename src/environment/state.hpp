#pragma once
#ifdef ENABLE_ROTATE_MODEL
#include <cstdint>
#include <vector>

struct State {
    int location = -1;
    int timestep = -1;
    int orientation = -1;

    struct Hasher {
        uint64_t operator()(const State &n) const;
    };

    bool operator==(const State &other) const;

    bool operator!=(const State &other) const;

    State() = default;

    State(int location, int timestep = -1, int orientation = -1);
};

using RobotPath = std::vector<State>;
#endif
