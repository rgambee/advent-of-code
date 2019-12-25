#include <array>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "utils.h"


using coord_type = std::array<int, 3>;
using grid_type = std::set<coord_type>;


/*   --
     21012
 -2  .....
 -1  .....
  0  .....
  1  .....
  2  .....
*/


std::vector<coord_type> get_neighbors(coord_type coords, bool recursive = false) {
    std::vector<coord_type> neighbors;
    // Add neighbors on own level
    for (auto dx: {-1, 1}) {
        auto x = coords[0] + dx;
        if (x < -2 || x > 2) {
            continue;
        }
        if (!(recursive && x == 0 && coords[1] == 0)) {
            // Only add (0, 0, z) if we're not in recursive mode
            neighbors.push_back({x, coords[1], coords[2]});
        }
    }
    for (auto dy: {-1, 1}) {
        auto y = coords[1] + dy;
        if (y < -2 || y > 2) {
            continue;
        }
        if (!(recursive && coords[0] == 0 &&  y == 0)) {
            // Only add (0, 0, z) if we're not in recursive mode
            neighbors.push_back({coords[0], y, coords[2]});
        }
    }
    if (!recursive) {
        return neighbors;
    }

    // Add neighbors on other levels
    switch (coords[0]) {
        case -2:
            // Left outer edge
            neighbors.push_back({-1, 0, coords[2] - 1});
            break;
        case 2:
            // Right outer edge
            neighbors.push_back({1, 0, coords[2] - 1});
            break;
        case -1:
            // Left inner edge
            if (coords[1] == 0) {
                neighbors.push_back({-2, -2, coords[2] + 1});
                neighbors.push_back({-2, -1, coords[2] + 1});
                neighbors.push_back({-2,  0, coords[2] + 1});
                neighbors.push_back({-2,  1, coords[2] + 1});
                neighbors.push_back({-2,  2, coords[2] + 1});
            }
            break;
        case 1:
            // Right inner edge
            if (coords[1] == 0) {
                neighbors.push_back({2, -2, coords[2] + 1});
                neighbors.push_back({2, -1, coords[2] + 1});
                neighbors.push_back({2,  0, coords[2] + 1});
                neighbors.push_back({2,  1, coords[2] + 1});
                neighbors.push_back({2,  2, coords[2] + 1});
            }
    }
    switch (coords[1]) {
        case -2:
            // Top outer edge
            neighbors.push_back({0, -1, coords[2] - 1});
            break;
        case 2:
            // Bottom outer edge
            neighbors.push_back({0, 1, coords[2] - 1});
            break;
        case -1:
            // Top inner edge
            if (coords[0] == 0) {
                neighbors.push_back({-2, -2, coords[2] + 1});
                neighbors.push_back({-1, -2, coords[2] + 1});
                neighbors.push_back({ 0, -2, coords[2] + 1});
                neighbors.push_back({ 1, -2, coords[2] + 1});
                neighbors.push_back({ 2, -2, coords[2] + 1});
            }
            break;
        case 1:
            if (coords[0] == 0) {
                neighbors.push_back({-2, 2, coords[2] + 1});
                neighbors.push_back({-1, 2, coords[2] + 1});
                neighbors.push_back({ 0, 2, coords[2] + 1});
                neighbors.push_back({ 1, 2, coords[2] + 1});
                neighbors.push_back({ 2, 2, coords[2] + 1});
            }
    }
    return neighbors;
}


size_t count_occupied_neighbors(const grid_type &grid,
                                const std::vector<coord_type> &neighbors) {
    size_t occupied = 0;
    for (auto &neigh: neighbors) {
        occupied += grid.find(neigh) != grid.end() ? 1 : 0;
    }
    return occupied;
}


grid_type update_grid(const grid_type &current_grid, bool recursive = false) {
    // Enable grid to spread
    grid_type new_grid;
    for (auto &coords: current_grid) {
        auto neighbors = get_neighbors(coords, recursive);
        auto num_occupied_neighbors = count_occupied_neighbors(
            current_grid, neighbors);
        if (num_occupied_neighbors == 1) {
            // This bug survives into the next minute
            new_grid.insert(coords);
        }
        for (auto &neigh: neighbors) {
            auto neigh_occupied = current_grid.find(neigh) != current_grid.end();
            if (neigh_occupied) {
                // Since this cell is occupied, we will get to it
                // (or have already gotten to it) in the outer for loop
                continue;
            }
            auto neigh_occupied_neighbors = count_occupied_neighbors(
                current_grid, get_neighbors(neigh, recursive));
            if (neigh_occupied_neighbors == 1 || neigh_occupied_neighbors == 2) {
                new_grid.insert(neigh);
            }
        }
    }
    return new_grid;
}


void print_grid(const grid_type &grid, int depth = 0) {
    for (auto row = -2; row <= 2; ++row) {
        for (auto col = -2; col <= 2; ++col) {
            coord_type coords{col, row, depth};
            std::cout << (grid.find(coords) != grid.end() ? '#' : '.');
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


long long calculate_biodiversity(const grid_type &grid, int depth = 0) {
    long long rating = 0, mult = 1;
    for (auto row = -2; row <= 2; ++row) {
        for (auto col = -2; col <= 2; ++col, mult *= 2) {
            coord_type coords{col, row, depth};
            rating += grid.find(coords) != grid.end() ? mult : 0;
        }
    }
    return rating;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    grid_type grid;
    std::string line;
    int row = -2;
    while (std::getline(input_stream, line)) {
        for (int col = -2; col < static_cast<int>(line.size()); ++col) {
            if (line[col + 2] == '#') {
                grid.insert({col, row, 0});
            }
        }
        ++row;
    }

    // print_grid(grid);
    grid_type orig_grid = grid;

    std::set<std::set<coord_type> > seen_grids;
    while (true) {
        if (seen_grids.find(grid) != seen_grids.end()) {
            break;
        }
        seen_grids.insert(grid);
        grid = update_grid(grid);
        // print_grid(grid);
    }
    // print_grid(grid);
    auto rating = calculate_biodiversity(grid);

    // Reset grid to its original state for part 2
    grid = orig_grid;
    for (auto min = 0; min < 200; ++min) {
        grid = update_grid(grid, true);
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Biodiversity rating: " << rating << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Number of bugs after 200 minutes: " << grid.size() << std::endl;
    return 0;
}
