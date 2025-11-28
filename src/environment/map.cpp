#include <environment/map.hpp>

#include <utils/assert.hpp>
#include <utils/dsu.hpp>

uint32_t Map::get_rows() const {
    return rows;
}

uint32_t Map::get_cols() const {
    return cols;
}

uint32_t Map::get_size() const {
    return rows * cols + 1;
}

bool Map::is_free(uint32_t pos) const {
    ASSERT(pos < map.size(), "invalid pos");
    return map[pos];
}

std::istream &operator>>(std::istream &input, Map &map) {
    ASSERT(input, "unable to read");
    std::string line;
    std::getline(input, line);
    ASSERT(line == "type octile", "unexpected map file format");
    input >> line;
    ASSERT(line == "height", "unexpected map file format");
    input >> map.rows;
    input >> line;
    ASSERT(line == "width", "unexpected map file format");
    input >> map.cols;
    input >> line;
    ASSERT(line == "map", "unexpected map file format");
    map.map.assign(map.rows * map.cols + 1, false);
    for (uint32_t row = 0; row < map.rows; row++) {
        input >> line;
        ASSERT(line.size() == map.cols, "unexpected map file format");
        for (uint32_t col = 0; col < map.cols; col++) {
            ASSERT(line[col] == '.' || line[col] == '@' || line[col] == 'T', "unexpected cell type");
            map.map[row * map.cols + col + 1] = (line[col] == '.');
        }
    }

    // Populate free_locations
    for (uint32_t pos = 1; pos < map.get_size(); pos++) {
        if (map.is_free(pos)) {
            map.free_locations.push_back(pos);
        }
    }

    return input;
}

uint32_t Map::sample_free_location(Randomizer &rnd) const {
    ASSERT(!free_locations.empty(), "no free locations available to sample");
    uint32_t index = rnd.get(0, free_locations.size() - 1);
    return free_locations[index];
}


Map &get_map() {
    static Map map;
    return map;
}
