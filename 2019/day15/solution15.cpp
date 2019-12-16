#include <array>
#include <iostream>
#include <string>
#include <unordered_map>

#include "intcode.h"
#include "utils.h"


enum class Direction {
    NORTH = 1,
    SOUTH = 2,
    WEST = 3,
    EAST = 4
};

enum class Status {
    BLOCKED = 0,
    MOVED = 1,
    O2_SYSTEM = 2
};

enum class Cell {
    UNKNOWN = ' ',
    OPEN = '.',
    WALL = '#',
    DROID = 'D',
    O2_SYSTEM = 'O'
};

constexpr int ESTIMATED_MAZE_SIZE = 50;
// Order of coordinates is (y, x) for drawing simplicity
using coord_type = std::array<int, 2>;
using grid_type = std::array<std::array<Cell, ESTIMATED_MAZE_SIZE>, ESTIMATED_MAZE_SIZE>;
const std::unordered_map<Direction, Direction> TURN_LEFT = {
    {Direction::NORTH, Direction::WEST}, {Direction::WEST, Direction::SOUTH},
    {Direction::SOUTH, Direction::EAST}, {Direction::EAST, Direction::NORTH}
};
const std::unordered_map<Direction, Direction> TURN_RIGHT = {
    {Direction::NORTH, Direction::EAST}, {Direction::EAST, Direction::SOUTH},
    {Direction::SOUTH, Direction::WEST}, {Direction::WEST, Direction::NORTH}
};


class Droid {
public:
    Droid(coord_type pos): position(pos) {}

    void move(Direction dir) {
        switch (dir) {
            case Direction::NORTH:
                position = coord_type{position[0] - 1, position[1]};
                break;
            case Direction::SOUTH:
                position = coord_type{position[0] + 1, position[1]};
                break;
            case Direction::WEST:
                position = coord_type{position[0], position[1] - 1};
                break;
            case Direction::EAST:
                position = coord_type{position[0], position[1] + 1};
                break;
        }
    }

    coord_type position;
    Direction facing = Direction::NORTH;
};


Direction get_direction_from_user() {
    std::cout << "Use wasd to move around" << std::endl;
    std::string input;
    std::cin >> input;
    if (input == "w") {
        return Direction::NORTH;
    } else if (input == "a") {
        return Direction::WEST;
    } else if (input == "s") {
        return Direction::SOUTH;
    } else if (input == "d") {
        return Direction::EAST;
    } else {
        std::cout << "Unrecognized input" << std::endl;
        return get_direction_from_user();
    }
}


// Wall-following algorithm
Direction pick_direction_automatically(const grid_type &grid,
                                       Droid &droid) {
    auto current_coords = droid.position;
    auto left = TURN_LEFT.at(droid.facing);
    droid.move(left);
    auto left_cell = grid[droid.position[0]][droid.position[1]];
    droid.position = current_coords;
    auto result = droid.facing;
    if (left_cell == Cell::OPEN) {
        droid.facing = left;
        result = left;
    } else if (left_cell == Cell::UNKNOWN) {
        result = left;
    } else if (left_cell == Cell::WALL) {
        droid.move(droid.facing);
        auto forward_cell = grid[droid.position[0]][droid.position[1]];
        droid.position = current_coords;
        if (forward_cell == Cell::WALL) {
            droid.facing = TURN_RIGHT.at(droid.facing);
        }
        result = droid.facing;
    }
    return result;
}


void draw_grid(const grid_type &grid, const Droid &droid) {
    for (auto y = 0; y < static_cast<int>(grid.size()); ++y) {
        for (auto x = 0; x < static_cast<int>(grid[y].size()); ++x) {
            if (coord_type{y, x} == droid.position) {
                std::cout << static_cast<char>(Cell::DROID);
            } else {
                std::cout << static_cast<char>(grid[y][x]);
            }
        }
        std::cout << std::endl;
    }
}


void update_maze(grid_type &grid, Droid &droid,
                 Direction attempted_direction, intcode_type reply) {
    switch (static_cast<Status>(reply)) {
        case Status::BLOCKED: {
            coord_type current_coords = droid.position;
            droid.move(attempted_direction);
            // std::cout << "Setting (" << droid.position[0] << ", ";
            // std::cout << droid.position[1] << ") to wall" << std::endl;
            grid[droid.position[0]][droid.position[1]] = Cell::WALL;
            droid.position = current_coords;
            break;
        }
        case Status::MOVED:
            droid.move(attempted_direction);
            // std::cout << "Setting (" << droid.position[0] << ", ";
            // std::cout << droid.position[1] << ") to open" << std::endl;
            grid[droid.position[0]][droid.position[1]] = Cell::OPEN;
            break;
        case Status::O2_SYSTEM:
            droid.move(attempted_direction);
            grid[droid.position[0]][droid.position[1]] = Cell::O2_SYSTEM;
            break;
        default:
            std::cerr << "Invalid intcode output: " << reply << std::endl;
            exit(3);
    }
    draw_grid(grid, droid);
    if (static_cast<Status>(reply) == Status::O2_SYSTEM) {
        std::cout << "Found O2 system" << std::endl;
    }
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    // Create empty grid
    grid_type grid;
    for (auto &row: grid) {
        for (auto &cell: row) {
            cell = Cell::UNKNOWN;
        }
    }
    // Put droid in center
    Droid droid(coord_type{grid.size()/2, grid.size()/2});
    grid[droid.position[0]][droid.position[1]] = Cell::OPEN;
    draw_grid(grid, droid);

    Direction attempted_direction;
    auto intcode_input = [&]() -> intcode_type {
        attempted_direction = pick_direction_automatically(grid, droid);
        return static_cast<intcode_type>(attempted_direction);
    };

    auto intcode_output = [&](intcode_type reply) {
        update_maze(grid, droid, attempted_direction, reply);
    };

    run_intcode_program(program, intcode_input, intcode_output);


    // std::cout << "PART 1" << std::endl;
    // std::cout << std::endl;
    // std::cout << "PART 2" << std::endl;
    return 0;
}
