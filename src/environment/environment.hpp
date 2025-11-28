#pragma once

#include <vector>
#include <environment/state.hpp>

struct Environment {
    int num_of_agents = 0;
    int rows = 0;
    int cols = 0;
    std::vector<int> map;

    std::vector<std::vector<int>> goal_locations;

    int curr_timestep = 0;
    std::vector<State> curr_states;
};
