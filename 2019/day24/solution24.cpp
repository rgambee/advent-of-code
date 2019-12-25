#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
// #include <valarray>
#include <vector>

#include "utils.h"


enum class Occupied {
    EMPTY = '.',
    BUG = '#'
};


using coord_type = std::array<size_t, 2>;
using grid_type = std::map<coord_type, Occupied>;


std::array<coord_type, 4> get_neighbors(coord_type coords) {
    return std::array<coord_type, 4>{
        coord_type{coords[0], coords[1] + 1},
        coord_type{coords[0], coords[1] - 1},
        coord_type{coords[0] + 1, coords[1]},
        coord_type{coords[0] - 1, coords[1]},
    };
}


grid_type update_grid(const grid_type &current_grid) {
    grid_type new_grid;
    for (auto &pair: current_grid) {
        auto coords = pair.first;
        auto contents = pair.second;
        auto num_occupied_neighbors = 0;
        for (auto &neigh: get_neighbors(coords)) {
            if (current_grid.find(neigh) != current_grid.end()
                && current_grid.at(neigh) == Occupied::BUG) {
                ++num_occupied_neighbors;
            }
        }
        if (contents == Occupied::BUG) {
            new_grid[coords] = num_occupied_neighbors == 1 ? Occupied::BUG : Occupied::EMPTY;
        } else if (num_occupied_neighbors == 1
                   || num_occupied_neighbors == 2) {
            new_grid[coords] = Occupied::BUG;
        } else {
            new_grid[coords] = Occupied::EMPTY;
        }
    }
    return new_grid;
}


std::vector<Occupied> grid_to_vector(const grid_type &grid) {
    std::vector<Occupied> vec(grid.size());
    size_t i = 0;
    for (auto iter = grid.begin(); iter != grid.end(); ++iter, ++i) {
        vec[i] = iter->second;
    }
    return vec;
}


void print_grid(const grid_type &grid) {
    auto i = 0;
    for (auto iter = grid.begin(); iter != grid.end(); ++iter, ++i) {
        std::cout << static_cast<char>(iter->second);
        if (i % 5 == 4) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    grid_type grid;
    std::string line;
    size_t row = 0;
    while (std::getline(input_stream, line)) {
        for (size_t col = 0; col < line.size(); ++col) {
            grid[coord_type{row, col}] = static_cast<Occupied>(line[col]);
        }
        ++row;
    }

    std::set<std::vector<Occupied> > seen;
    std::vector<Occupied> state;
   while (true) {
        state = grid_to_vector(grid);
        if (seen.find(state) != seen.end()) {
            break;
        }
        seen.insert(state);
        grid = update_grid(grid);
    }

    long long rating = 0;
    for (size_t i = 0; i < state.size(); ++i) {
        auto x = state[i] == Occupied::BUG ? 1 : 0;
        rating += x * std::pow(2ll, i);
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Biodiversity rating: " << rating << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
