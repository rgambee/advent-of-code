#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils.h"


using coord_type = std::array<size_t, 2>;

class Node {
public:
    Node(coord_type crds): coords(crds) {}

    void add_neighbor(std::shared_ptr<Node> &neigh) {
        neighbors.push_back(neigh);
    }

    coord_type coords{0, 0};
    std::map<size_t, size_t> distances;
    std::vector<std::shared_ptr<Node> > neighbors;
};


using node_ptr = std::shared_ptr<Node>;
using portals_type = std::unordered_map<std::string, std::vector<node_ptr> >;
using grid_type = std::map<coord_type, node_ptr>;
using node_depth_pair = std::pair<node_ptr, size_t>;


// TODO: reduce reused code
void find_outside_portals(const std::vector<std::string> &maze_str,
                          grid_type &grid, portals_type &portals,
                          size_t min_x, size_t min_y, size_t max_x, size_t max_y) {
    // Top
    for (size_t x = min_x, y = min_y; x <= max_x; ++x) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y - 2][x], maze_str[y - 1][x]};
        portals[portal_name].push_back(iter->second);
    }
    // Right
    for (size_t x = max_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x + 1], maze_str[y][x + 2]};
        portals[portal_name].push_back(iter->second);
    }
    // Bottom
    for (size_t x = min_x, y = max_y; x <= max_x; ++x) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y + 1][x], maze_str[y + 2][x]};
        portals[portal_name].push_back(iter->second);
    }
    // Left
    for (size_t x = min_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x - 2], maze_str[y][x - 1]};
        portals[portal_name].push_back(iter->second);
    }
}


void find_inside_portals(const std::vector<std::string> &maze_str,
                         grid_type &grid, portals_type &portals,
                         size_t min_x, size_t min_y, size_t max_x, size_t max_y) {
    // Top
    for (size_t x = min_x, y = min_y; x <= max_x; ++x) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y + 1][x], maze_str[y + 2][x]};
        portals[portal_name].push_back(iter->second);
    }
    // Right
    for (size_t x = max_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x - 2], maze_str[y][x - 1]};
        portals[portal_name].push_back(iter->second);
    }
    // Bottom
    for (size_t x = min_x, y = max_y; x <= max_x; ++x) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y - 2][x], maze_str[y - 1][x]};
        portals[portal_name].push_back(iter->second);
    }
    // Left
    for (size_t x = min_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x + 1], maze_str[y][x + 2]};
        portals[portal_name].push_back(iter->second);
    }
}


size_t solve_maze(node_ptr start, node_ptr end,
                  std::function<bool(coord_type)> on_outside_edge,
                  std::function<bool(coord_type)> on_inside_edge,
                  bool recursive = false) {
    // Breadth-first search
    start->distances[0] = 0;
    std::queue<node_depth_pair> active_nodes;
    active_nodes.emplace(start, 0);
    auto found_end = false;
    while (!found_end && !active_nodes.empty()) {
        auto current = active_nodes.front();
        active_nodes.pop();
        auto curr_node = current.first;
        auto curr_depth = current.second;
        for (auto neigh: curr_node->neighbors) {
            auto neigh_depth = curr_depth;
            if (recursive && on_outside_edge(curr_node->coords)
                && on_inside_edge(neigh->coords)) {
                // Moveing outside to in goes up one level
                if (curr_depth == 0) {
                    // Outside portals act like walls at top level
                    continue;
                } else {
                    neigh_depth = curr_depth - 1;
                }
            } else if (recursive && on_inside_edge(curr_node->coords)
                       && on_outside_edge(neigh->coords)) {
                // Moving inside to out goes down one level
                neigh_depth = curr_depth + 1;
            }
            if (neigh->distances.find(neigh_depth) == neigh->distances.end()) {
                neigh->distances[neigh_depth] = curr_node->distances.at(curr_depth) + 1;
                if (neigh->coords == end->coords && neigh_depth == 0) {
                    found_end = true;
                    break;
                } else {
                    active_nodes.emplace(neigh, neigh_depth);
                }
            }
        }
    }
    return end->distances.at(0);
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    // Read in maze as vector of strings
    std::vector<std::string> maze_str;
    std::string line;
    while (std::getline(input_stream, line)) {
        maze_str.push_back(line);
    }

    // Convert to grid of connected Nodes (no portals yet)
    grid_type grid;
    size_t min_x = std::numeric_limits<size_t>::max();
    size_t min_y = std::numeric_limits<size_t>::max();
    size_t max_x = std::numeric_limits<size_t>::min();
    size_t max_y = std::numeric_limits<size_t>::min();
    for (size_t y = 0; y < maze_str.size(); ++y) {
        for (size_t x = 0; x < maze_str[y].size(); ++x) {
            if (maze_str[y][x] == '.') {
                auto new_node = std::make_shared<Node>(coord_type{x, y});
                grid[coord_type{x, y}] = new_node;
                auto left = grid.find(coord_type{x - 1, y});
                if (left != grid.end()) {
                    left->second->add_neighbor(new_node);
                    new_node->add_neighbor(left->second);
                }
                auto above = grid.find(coord_type{x, y - 1});
                if (above != grid.end()) {
                    above->second->add_neighbor(new_node);
                    new_node->add_neighbor(above->second);
                }
                min_x = std::min(min_x, x);
                min_y = std::min(min_y, y);
                max_x = std::max(max_x, x);
                max_y = std::max(max_y, y);
            }
        }
    }

    auto on_outside_edge = [min_x, min_y, max_x, max_y](coord_type coords) {
        return (coords[0] == min_x || coords[0] == max_x
                || coords[1] == min_y || coords[1] == max_y);
    };

    // Walk around the outside adding portals
    portals_type portals;
    find_outside_portals(maze_str, grid, portals, min_x, min_y, max_x, max_y);

    // Start in the center and move outward to find the inner boundaries
    auto center_x = (min_x + max_x) / 2, center_y = (min_y + max_y) / 2;
    // Up
    for (size_t y = center_y; y > 0; --y) {
        if (maze_str[y][center_x] == '#' || maze_str[y][center_x] == '.') {
            min_y = y;
            break;
        }
    }
    // Right
    for (size_t x = center_x; x <= maze_str[center_y].size(); ++x) {
        if (maze_str[center_y][x] == '#' || maze_str[center_y][x] == '.') {
            max_x = x;
            break;
        }
    }
    // Down
    for (size_t y = center_y; y < maze_str.size(); ++y) {
        if (maze_str[y][center_x] == '#' || maze_str[y][center_x] == '.') {
            max_y = y;
            break;
        }
    }
    // Left
    for (size_t x = center_x; x > 0; --x) {
        if (maze_str[center_y][x] == '#' || maze_str[center_y][x] == '.') {
            min_x = x;
            break;
        }
    }

    auto on_inside_edge = [min_x, min_y, max_x, max_y](coord_type coords) {
        return (coords[0] == min_x || coords[0] == max_x
                || coords[1] == min_y || coords[1] == max_y);
    };

    // Walk around the inside
    find_inside_portals(maze_str, grid, portals, min_x, min_y, max_x, max_y);

    // Link portal nodes together
    for (auto port_iter = portals.begin(); port_iter != portals.end(); ++port_iter) {
        for (size_t i = 0; i < port_iter->second.size(); ++i) {
            for (size_t j = 0; j < port_iter->second.size(); ++j) {
                if (i == j) {
                    continue;
                }
                port_iter->second[i]->add_neighbor(port_iter->second[j]);
                port_iter->second[j]->add_neighbor(port_iter->second[i]);
            }
        }
    }

    auto start = portals.at("AA")[0];
    auto end = portals.at("ZZ")[0];
    auto part1_answer = solve_maze(start, end, on_outside_edge, on_inside_edge, false);
    // Clear all node distances
    for (auto &pair: grid) {
        pair.second->distances.clear();
    }
    auto part2_answer = solve_maze(start, end, on_outside_edge, on_inside_edge, true);


    std::cout << "PART 1" << std::endl;
    std::cout << "Non-recursive distance to end: " << part1_answer << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Recursive distance to end: " << part2_answer << std::endl;
    return 0;
}
