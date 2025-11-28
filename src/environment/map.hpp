#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include <utils/randomizer.hpp>

// contains information about map:
// rows, cols, is_free
class Map {
    uint32_t rows = 0;
    uint32_t cols = 0;

    // false: obstacle, true: free
    std::vector<bool> map;

    std::vector<uint32_t> free_locations;

public:
    Map() = default;

    [[nodiscard]] uint32_t get_rows() const;

    [[nodiscard]] uint32_t get_cols() const;

    [[nodiscard]] uint32_t get_size() const;

    [[nodiscard]] bool is_free(uint32_t pos) const;

    [[nodiscard]] const std::vector<uint32_t> &get_free_locations() const {
        return free_locations;
    }

    [[nodiscard]] uint32_t sample_free_location(Randomizer &rnd) const;

    friend std::istream &operator>>(std::istream &input, Map &map);
};

Map &get_map();
