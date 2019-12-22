#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <queue>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
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
    int distance = -1;
    std::vector<std::shared_ptr<Node> > neighbors;
};


using node_ptr = std::shared_ptr<Node>;
using portals_type = std::unordered_map<std::string, std::vector<node_ptr> >;
using grid_type = std::map<coord_type, node_ptr>;


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
        std::cout << "Adding " << portal_name << std::endl;
        portals[portal_name].push_back(iter->second);
    }
    // Right
    for (size_t x = max_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x + 1], maze_str[y][x + 2]};
        std::cout << "Adding " << portal_name << std::endl;
        portals[portal_name].push_back(iter->second);
    }
    // Bottom
    for (size_t x = min_x, y = max_y; x <= max_x; ++x) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y + 1][x], maze_str[y + 2][x]};
        std::cout << "Adding " << portal_name << std::endl;
        portals[portal_name].push_back(iter->second);
    }
    // Left
    for (size_t x = min_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x - 2], maze_str[y][x - 1]};
        std::cout << "Adding " << portal_name << std::endl;
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
        std::cout << "Adding " << portal_name << std::endl;
        portals[portal_name].push_back(iter->second);
    }
    // Right
    for (size_t x = max_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x - 2], maze_str[y][x - 1]};
        std::cout << "Adding " << portal_name << std::endl;
        portals[portal_name].push_back(iter->second);
    }
    // Bottom
    for (size_t x = min_x, y = max_y; x <= max_x; ++x) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y - 2][x], maze_str[y - 1][x]};
        std::cout << "Adding " << portal_name << std::endl;
        portals[portal_name].push_back(iter->second);
    }
    // Left
    for (size_t x = min_x, y = min_y; y <= max_y; ++y) {
        auto iter = grid.find(coord_type{x, y});
        if (iter == grid.end()) {
            continue;
        }
        std::string portal_name{maze_str[y][x + 1], maze_str[y][x + 2]};
        std::cout << "Adding " << portal_name << std::endl;
        portals[portal_name].push_back(iter->second);
    }
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

    // Solve flat maze using breadth-first search
    auto start = portals.at("AA")[0];
    start->distance = 0;
    auto end = portals.at("ZZ")[0];
    std::queue<node_ptr> active_nodes;
    active_nodes.push(start);
    while (!active_nodes.empty()) {
        auto current = active_nodes.front();
        active_nodes.pop();
        for (auto neigh: current->neighbors) {
            if (neigh->distance < 0) {
                neigh->distance = current->distance + 1;
                active_nodes.push(neigh);
            }
        }
    }


    std::cout << "PART 1" << std::endl;
    std::cout << "Steps from AA to ZZ: " << end->distance << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
