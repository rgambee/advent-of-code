#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <map>

#include "intcode.h"
#include "utils.h"


enum class Status {
    STATIONARY = 0,
    PULLED = 1
};

using coord_type = std::array<size_t, 2>;
using grid_type = std::map<coord_type, Status>;
constexpr size_t SCAN_SIZE = 50;
constexpr size_t SHIP_SIZE = 100;


Status get_droid_status(const program_type &program, coord_type coords) {
    size_t index = 0;
    Status result;
    auto intcode_input = [&coords, &index]() -> intcode_type {
        return coords[index++];
    };
    auto intcode_output = [&result](intcode_type output) -> void {
        result = static_cast<Status>(output);
    };
    run_intcode_program(program, intcode_input, intcode_output);
    return result;
}


Status get_from_grid(grid_type &grid, coord_type coords,
                     const program_type &program) {
    if (grid.find(coords) == grid.end()) {
        grid[coords] = get_droid_status(program, coords);
    }
    return grid.at(coords);
}


void draw_grid(const grid_type &grid) {
    for (size_t i = 0; i < SCAN_SIZE; ++i) {
        for (size_t j = 0; j < SCAN_SIZE; ++j) {
            std::cout << (grid.at(coord_type{j, i}) == Status::PULLED ? '#' : '.');
        }
        std::cout << std::endl;
    }
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    grid_type grid;
    size_t affected = 0, part1_answer = 0, part2_answer = 0;

    for (size_t x = 0; x < SCAN_SIZE; ++x) {
        for (size_t y = 0; y < SCAN_SIZE; ++y) {
            if (get_from_grid(grid, coord_type{x, y}, program) == Status::PULLED) {
                ++affected;
            }
            if (x == SCAN_SIZE - 1 && y == SCAN_SIZE - 1) {
                part1_answer = affected;
            }
        }
    }
    draw_grid(grid);

    // Walk along the outer edges of the beam
    // FIXME: generalize the starting point, don't used hard-coded numbers
    coord_type left_edge{13, 17}, right_edge{13, 17};
    while (true) {
        auto x_width = right_edge[0] - left_edge[0] + 1;
        auto y_width = left_edge[1] - right_edge[1] + 1;
        if (x_width >= SHIP_SIZE && y_width >= SHIP_SIZE) {
            part2_answer = left_edge[0] * 10000 + right_edge[1];
            break;
        }
        if (x_width < y_width) {
            ++right_edge[1];
            while (get_from_grid(grid, right_edge, program) == Status::PULLED) {
                ++right_edge[0];
            }
            --right_edge[0];
        } else {
            ++left_edge[1];
            while (get_from_grid(grid, left_edge, program) == Status::STATIONARY) {
                ++left_edge[0];
            }
        }
    }


    std::cout << "PART 1" << std::endl;
    std::cout << "Number of affected points: " << part1_answer << std::endl;
    std::cout << std::endl;
    std::cout << "Nearest coordinates to fit ship: " << part2_answer << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
