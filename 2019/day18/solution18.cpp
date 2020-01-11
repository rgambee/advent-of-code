#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>

#include "utils.h"


using coord_type = std::array<int, 2>;
using key_type = char;


enum class TileType {
    FLOOR = '.',
    WALL = '#',
    START = '@',
    DOOR,
    KEY
};


bool is_door(char c) {
    // Doors are upper-case letters
    return 'A' <= c && c <= 'Z';
}


bool is_key(char c) {
    // Keys are lower-case letters
    return 'a' <= c && c <= 'z';
}


char key_to_door(char k) {
    if (!is_key(k)) {
        std::stringstream error_message;
        error_message << "Not a key: " << k;
        throw std::invalid_argument(error_message.str());
    }
    return k + ('A' - 'a');
}


char door_to_key(char d) {
    if (!is_door(d)) {
        std::stringstream error_message;
        error_message << "Not a door: " << d;
        throw std::invalid_argument(error_message.str());
    }
    return d + ('a' - 'A');
}


class Node {
public:
    Node(coord_type c): coords(c) {}
    Node(coord_type c, key_type k): coords(c), required_key(k) {}

    bool passable(const std::set<key_type> &keys) const {
        if (required_key == 0) {
            return true;
        }
        return keys.find(required_key) != keys.end();
    }

    const coord_type coords{0, 0};
    int distance = -1;
    const key_type required_key = 0;
};

using grid_type = std::map<coord_type, std::shared_ptr<Node> >;


std::vector<coord_type> get_neighbors(const grid_type &grid,
                                      const coord_type &coords) {
    std::vector<coord_type> neighbors;
    for (auto &delta: {{1, 0}, {0, 1}, {-1, 0}, {0, -1}}) {
        coord_type possible_neighbor{coords[0] + delta[0], coords[1] + delta[1]};
        if (grid.find(possible_neighbor) != grid.end()) {
            neighbors.push_back(possible_neighbor);
        }
    }
    return neighbors;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);

    std::set<key_type> keys;

    return 0;
}
