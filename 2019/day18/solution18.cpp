#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>

#include "utils.h"


using coord_type = std::array<int, 2>;
using key_type = char;
using door_type = char;
using reachable_type = std::tuple<coord_type, size_t, size_t>;
using memo_key_type = std::tuple<std::vector<coord_type>, std::set<key_type> >;


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


bool is_door(TileType t) {
    return is_door(static_cast<char>(t));
}

bool is_key(char c) {
    // Keys are lower-case letters
    return 'a' <= c && c <= 'z';
}

bool is_key(TileType t) {
    return is_key(static_cast<char>(t));
}


door_type key_to_door(key_type k) {
    if (!is_key(static_cast<char>(k))) {
        std::stringstream error_message;
        error_message << "Not a key: " << k;
        throw std::invalid_argument(error_message.str());
    }
    return k + ('A' - 'a');
}


key_type door_to_key(door_type d) {
    if (!is_door(static_cast<char>(d))) {
        std::stringstream error_message;
        error_message << "Not a door: " << d;
        throw std::invalid_argument(error_message.str());
    }
    return d + ('a' - 'A');
}


class Node {
public:
    Node(TileType t, coord_type c): tile_type(t), coords(c) {}

    bool passable(const std::set<key_type> &keys) const {
        if (tile_type == TileType::FLOOR || is_key(tile_type)) {
            return true;
        }
        if (!is_door(tile_type)) {
            std::stringstream error_message;
            error_message << "Unexpected tile type " << static_cast<char>(tile_type);
            throw std::logic_error(error_message.str());
        }
        return keys.find(door_to_key(static_cast<door_type>(tile_type))) != keys.end();
    }

    TileType tile_type = TileType::FLOOR;
    const coord_type coords{0, 0};
    int distance = -1;
};


using node_ptr = std::shared_ptr<Node>;
using grid_type = std::map<coord_type, node_ptr>;


std::vector<coord_type> get_neighbors(const grid_type &grid,
                                      const coord_type &coords) {
    std::vector<coord_type> neighbors;
    for (auto &delta: {coord_type{1, 0}, coord_type{0, 1},
                       coord_type{-1, 0}, coord_type{0, -1}}) {
        coord_type possible_neighbor{coords[0] + delta[0], coords[1] + delta[1]};
        if (grid.find(possible_neighbor) != grid.end()) {
            neighbors.push_back(possible_neighbor);
        }
    }
    return neighbors;
}


void reset_distances(const grid_type &grid) {
    for (auto &pair: grid) {
        pair.second->distance = -1;
    }
}


std::list<std::pair<coord_type, size_t> > distance_to_keys(
        const grid_type &grid,
        const coord_type start,
        const std::set<key_type> &held_keys) {
    reset_distances(grid);
    std::map<coord_type, size_t> keys_reached;
    grid.at(start)->distance = 0;
    std::vector<node_ptr> active_nodes{grid.at(start)};
    while (!active_nodes.empty()) {
        auto active = active_nodes.front();
        active_nodes.erase(active_nodes.begin());
        auto neighbor_coords = get_neighbors(grid, active->coords);
        for (auto &neigh: neighbor_coords) {
            auto neigh_node = grid.at(neigh);
            if (neigh_node->distance >= 0 || !neigh_node->passable(held_keys)) {
                continue;
            }
            neigh_node->distance = active->distance + 1;
            if (is_key(neigh_node->tile_type)) {
                if (keys_reached.find(neigh) != keys_reached.end()) {
                    throw std::logic_error("Found same key twice");
                }
                auto key = static_cast<key_type>(neigh_node->tile_type);
                if (held_keys.find(key) == held_keys.end()) {
                    keys_reached[neigh] = neigh_node->distance;
                }
            }
            active_nodes.push_back(neigh_node);
        }
    }
    return std::list<std::pair<coord_type, size_t> >(keys_reached.begin(),
                                                     keys_reached.end());
}


size_t score(const grid_type &grid,
             const std::vector<coord_type> &starts,
             const std::set<key_type> &held_keys,
             const size_t total_number_of_keys,
             std::map<memo_key_type, int> &memo) {
    if (held_keys.size() == total_number_of_keys) {
        return 0;
    }
    if (starts.empty()) {
        throw std::length_error("No starting location(s) provided");
    }
    auto memo_entry = std::make_tuple(starts, held_keys);
    if (memo.find(memo_entry) != memo.end()) {
        if (memo.at(memo_entry) >= 0) {
            std::cout << "Memo match" << std::endl;
            return memo.at(memo_entry);
        } else {
            // This state is already queued for evaluation.
            // Throw an error so we can postpone it for now.
            throw std::invalid_argument("Already evaluating memo entry");
        }
    }
    // Mark this memo entry as under evaluation
    memo[memo_entry] = -1;
    std::list<reachable_type> reachable_keys;
    for (size_t i = 0; i < starts.size(); ++i) {
        auto reachable_from_start = distance_to_keys(grid, starts[i], held_keys);
        for (auto &pair: reachable_from_start) {
            reachable_keys.emplace_back(pair.first, pair.second, i);
        }
    }
    // Sort keys by distance to their respective starting locations
    reachable_keys.sort([](reachable_type t1, reachable_type t2) -> bool {
                                return std::get<1>(t1) < std::get<1>(t2);
    });
    size_t best_score = std::numeric_limits<size_t>::max();
    while (!reachable_keys.empty()) {
        auto [key_coords, dist, start_index] = reachable_keys.front();
        reachable_keys.pop_front();
        auto key = static_cast<key_type>(grid.at(key_coords)->tile_type);
        if (held_keys.find(key) != held_keys.end()) {
            // Already have this key
            continue;
        }
        std::set<key_type> new_keys{held_keys};
        new_keys.insert(key);
        auto new_starts(starts);
        new_starts[start_index] = key_coords;
        try {
            // std::cout << "Evaluating" << std::endl;
            auto new_score = (score(grid, new_starts, new_keys,
                                    total_number_of_keys, memo)
                              + dist);
            if (new_score < best_score) {
                best_score = new_score;
            }
        } catch (const std::invalid_argument &err) {
            // Entry is already under evaluation, so push it to the end
            reachable_keys.emplace_back(key_coords, dist, start_index);
        }
    }
    if (best_score == std::numeric_limits<size_t>::max()) {
        throw std::runtime_error("Unable to reach all keys");
    }
    memo[memo_entry] = best_score;
    std::cout << memo.size() << std::endl;
    return best_score;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    std::string line;
    std::vector<coord_type> starts;
    grid_type grid;
    size_t total_number_of_keys = 0;
    for (size_t y = 0; std::getline(input_stream, line); ++y) {
        for (size_t x = 0; x < line.size(); ++x) {
            auto new_tile = static_cast<TileType>(line[x]);
            if (new_tile == TileType::WALL) {
                continue;
            }
            coord_type coords{static_cast<int>(x), static_cast<int>(y)};
            grid[coords] = std::make_shared<Node>(new_tile, coords);
            if (new_tile == TileType::START) {
                starts.push_back(coords);
                grid.at(coords)->tile_type = TileType::FLOOR;
            } else if (is_key(new_tile)) {
                ++total_number_of_keys;
            }
        }
    }
    if (starts.size() != 1) {
        throw std::runtime_error("Didn't find unique starting location");
    }

    std::set<key_type> keys;
    std::map<std::tuple<std::vector<coord_type>, std::set<key_type> >, int> score_memo;
    auto part1_result = score(grid, starts, keys, total_number_of_keys, score_memo);

    // Modify the maze for part 2
    for (auto x: {-1, 0, 1}) {
        for (auto y: {-1, 0, 1}) {
            coord_type coords{starts[0][0] + x, starts[0][1] + y};
            if (x == 0 || y == 0) {
                // Add a wall at this location (i.e. remove it from the grid)
                grid.erase(coords);
            } else if (!(x == 0 && y == 0)) {
                // Add this as a starting location
                starts.push_back(coords);
            }
        }
    }
    // Remove central starting location
    starts.erase(starts.begin());

    keys.clear();
    score_memo.clear();
    auto part2_result = score(grid, starts, keys, total_number_of_keys, score_memo);

    std::cout << "PART 1" << std::endl;
    std::cout << "Shortest path to collect all keys: " << part1_result << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Shortest path to collect all keys: " << part2_result << std::endl;
    return 0;
}
