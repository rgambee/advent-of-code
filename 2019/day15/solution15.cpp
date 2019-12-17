#include <array>
#include <iostream>
#include <list>
#include <set>
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


class Node {
public:
    Node() = default;
    Node(Cell ct): cell_type(ct) {}

    Cell cell_type = Cell::UNKNOWN;
    int distance = -1;
};


constexpr int ESTIMATED_MAZE_SIZE = 50;
constexpr int CENTER = ESTIMATED_MAZE_SIZE / 2;
// Order of coordinates is (y, x) for drawing simplicity
using coord_type = std::array<int, 2>;
using grid_type = std::array<std::array<Node, ESTIMATED_MAZE_SIZE>, ESTIMATED_MAZE_SIZE>;
const std::unordered_map<Direction, Direction> TURN_LEFT = {
    {Direction::NORTH, Direction::WEST}, {Direction::WEST, Direction::SOUTH},
    {Direction::SOUTH, Direction::EAST}, {Direction::EAST, Direction::NORTH}
};
const std::unordered_map<Direction, Direction> TURN_RIGHT = {
    {Direction::NORTH, Direction::EAST}, {Direction::EAST, Direction::SOUTH},
    {Direction::SOUTH, Direction::WEST}, {Direction::WEST, Direction::NORTH}
};


coord_type get_neighbor(const coord_type &coords, Direction dir) {
    switch (dir) {
        case Direction::NORTH:
            return coord_type{coords[0] - 1, coords[1]};
        case Direction::SOUTH:
            return coord_type{coords[0] + 1, coords[1]};
        case Direction::WEST:
            return coord_type{coords[0], coords[1] - 1};
        case Direction::EAST:
            return coord_type{coords[0], coords[1] + 1};
    }
    return coord_type{-1, -1};
}

class Droid {
public:
    Droid(coord_type pos): position(pos) {}

    void move(Direction dir) {
        position = get_neighbor(position, dir);
    }

    coord_type position;
    Direction facing = Direction::NORTH;
    bool done_exploring = false;
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
    auto left = TURN_LEFT.at(droid.facing);
    auto left_coords = get_neighbor(droid.position, left);
    auto &left_cell = grid[left_coords[0]][left_coords[1]];
    auto result = droid.facing;
    if (left_cell.cell_type == Cell::OPEN) {
        droid.facing = left;
        result = left;
    } else if (left_cell.cell_type == Cell::UNKNOWN) {
        result = left;
    } else if (left_cell.cell_type == Cell::WALL) {
        auto forward_coords = get_neighbor(droid.position, droid.facing);
        auto &forward_cell = grid[forward_coords[0]][forward_coords[1]];
        if (forward_cell.cell_type == Cell::WALL) {
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
                switch (droid.facing) {
                    case Direction::NORTH:
                        std::cout << "^";
                        break;
                    case Direction::SOUTH:
                        std::cout << "v";
                        break;
                    case Direction::WEST:
                        std::cout << "<";
                        break;
                    case Direction::EAST:
                        std::cout << ">";
                        break;
                }
            } else {
                std::cout << static_cast<char>(grid[y][x].cell_type);
            }
        }
        std::cout << std::endl;
    }
}


void clear_distances(grid_type &grid) {
    for (auto &row: grid) {
        for (auto &node: row) {
            node.distance = -1;
        }
    }
    if (grid[CENTER][CENTER].distance != -1) {
        std::cerr << "Uh-oh" << std::endl;
        exit(99);
    }
}


void find_distances(grid_type &grid, const coord_type &starting_point) {
    clear_distances(grid);
    grid[starting_point[0]][starting_point[1]].distance = 0;
    std::list<coord_type> active_coords{starting_point};
    std::array<Direction, 4> directions{
        Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST};
    while (active_coords.size() > 0) {
        auto curr_coords = active_coords.front();
        active_coords.pop_front();
        auto &curr_cell = grid[curr_coords[0]][curr_coords[1]];
        for (auto dir: directions) {
            auto neigh_coords = get_neighbor(curr_coords, dir);
            auto &neigh_cell = grid[neigh_coords[0]][neigh_coords[1]];
            if (neigh_cell.cell_type != Cell::WALL
                && neigh_cell.distance < 0) {
                neigh_cell.distance = curr_cell.distance + 1;
                active_coords.push_back(neigh_coords);
            }
        }
    }
}


void compute_distances(grid_type &grid) {
    // Find distances from center
    find_distances(grid, coord_type{CENTER, CENTER});
    coord_type o2_system_coords;
    auto distance_to_o2_system = 0;
    for (grid_type::size_type i = 0; i < grid.size(); ++i) {
        for (grid_type::size_type j = 0; j < grid[i].size(); ++j) {
            if (grid[i][j].cell_type == Cell::O2_SYSTEM) {
                o2_system_coords = coord_type{
                    static_cast<int>(i), static_cast<int>(j)};
                distance_to_o2_system = grid[i][j].distance;
            }
        }
    }
    find_distances(grid, o2_system_coords);
    auto furthest_distance_from_o2 = 0;
    for (auto &row: grid) {
        for (auto &cell: row) {
            furthest_distance_from_o2 = std::max(
                furthest_distance_from_o2, cell.distance);
        }
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Distance to O2 system: " << distance_to_o2_system << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Time to fill with O2: " << furthest_distance_from_o2;
    std::cout << " minutes" << std::endl;
    exit(0);
}


void update_maze(grid_type &grid, Droid &droid,
                 Direction attempted_direction, intcode_type reply,
                 int iteration) {
    auto reply_status = static_cast<Status>(reply);
    switch (reply_status) {
        case Status::BLOCKED: {
            auto new_coords = get_neighbor(droid.position, attempted_direction);
            auto &new_cell = grid[new_coords[0]][new_coords[1]];
            if (new_cell.cell_type == Cell::UNKNOWN) {
                droid.done_exploring = false;
            }
            new_cell.cell_type = Cell::WALL;
            break;
        }
        case Status::MOVED:
        case Status::O2_SYSTEM: {
            droid.move(attempted_direction);
            auto &new_cell = grid[droid.position[0]][droid.position[1]];
            if (new_cell.cell_type == Cell::UNKNOWN) {
                droid.done_exploring = false;
            }
            if (reply_status == Status::MOVED) {
                new_cell.cell_type = Cell::OPEN;
            } else if (reply_status == Status::O2_SYSTEM) {
                new_cell.cell_type = Cell::O2_SYSTEM;
            }
            break;
        }
        default:
            std::cerr << "Invalid intcode output: " << reply << std::endl;
            exit(3);
    }
    // draw_grid(grid, droid);
    // auto &curr_cell = grid[droid.position[0]][droid.position[1]];
    // if (droid.position[0] == CENTER && droid.position[1] == CENTER) {
    //     std::cout << "At center: ";
    //     for (auto dir: curr_cell.facing_when_visited) {
    //         std::cout << static_cast<int>(dir) << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // if (static_cast<Status>(reply) == Status::MOVED &&
    //     curr_cell.facing_when_visited.find(droid.facing) != curr_cell.facing_when_visited.end()) {
    //     // We're back where we started
    //     draw_grid(grid, droid);
    //     compute_distances(grid);
    // } else {
    //     curr_cell.facing_when_visited.insert(droid.facing);
    // }
    // if (droid.done_exploring) {
    //     // Done exploring the maze
    //     draw_grid(grid, droid);
    //     compute_distances(grid);
    // } else if (droid.position == coord_type{CENTER, CENTER}
    //            && droid.facing == Direction::WEST) {
    //     droid.done_exploring = true;
    // }
    if (iteration > 100000) {
        // FIXME: figure out when the droid's done exploring the maze
        draw_grid(grid, droid);
        compute_distances(grid);
    }
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    // Create grid full of unknown Nodes
    grid_type grid;
    // Put droid in center
    Droid droid(coord_type{CENTER, CENTER});
    grid[droid.position[0]][droid.position[1]].cell_type = Cell::OPEN;
    auto iteration = 0;

    Direction attempted_direction;
    auto intcode_input = [&]() -> intcode_type {
        attempted_direction = pick_direction_automatically(grid, droid);
        return static_cast<intcode_type>(attempted_direction);
    };

    auto intcode_output = [&](intcode_type reply) {
        update_maze(grid, droid, attempted_direction, reply, iteration);
        ++iteration;
    };

    run_intcode_program(program, intcode_input, intcode_output);

    return 0;
}
