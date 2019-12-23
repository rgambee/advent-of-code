#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <vector>

#include "intcode.h"
#include "utils.h"


enum class CellType {
    SCAFFOLD = '#',
    SPACE = '.'
};


enum class Direction {
    NORTH = '^',
    EAST = '>',
    SOUTH = 'v',
    WEST = '<'
};


const std::map<Direction, Direction> TURN_LEFT = {
    {Direction::NORTH, Direction::WEST}, {Direction::WEST, Direction::SOUTH},
    {Direction::SOUTH, Direction::EAST}, {Direction::EAST, Direction::NORTH}
};
const std::map<Direction, Direction> TURN_RIGHT = {
    {Direction::NORTH, Direction::EAST}, {Direction::EAST, Direction::SOUTH},
    {Direction::SOUTH, Direction::WEST}, {Direction::WEST, Direction::NORTH}
};


enum class Move {
    FORWARD = 'F',
    LEFT = 'L',
    RIGHT = 'R'
};


class Cell {
public:
    Cell() = default;
    Cell(CellType ct): type(ct) {}

    CellType type = CellType::SPACE;
    std::shared_ptr<Cell> north = nullptr;
    std::shared_ptr<Cell> east = nullptr;
    std::shared_ptr<Cell> south = nullptr;
    std::shared_ptr<Cell> west = nullptr;
};


using coord_type = std::array<size_t, 2>;
using cell_ptr = std::shared_ptr<Cell>;
using grid_type = std::map<coord_type, cell_ptr>;


class Robot {
public:
    Robot() = default;

    coord_type position{0, 0};
    Direction facing = Direction::NORTH;
};


coord_type get_coords_in_direction(const coord_type &coords,
                                   const Direction &dir) {
    switch (dir) {
        case Direction::NORTH:
            return coord_type{coords[0] - 1, coords[1]};
        case Direction::EAST:
            return coord_type{coords[0], coords[1] + 1};
        case Direction::SOUTH:
            return coord_type{coords[0] + 1, coords[1]};
        case Direction::WEST:
            return coord_type{coords[0], coords[1] - 1};
        default:
            std::cerr << "Unknown direction: " << static_cast<char>(dir) << std::endl;
            exit(5);
    }
}


void add_grid_cell(grid_type &grid,
                   Robot &robo,
                   char camera_output,
                   size_t row, size_t col) {
    auto new_cell_type = static_cast<CellType>(camera_output);
    if (new_cell_type != CellType::SCAFFOLD && new_cell_type != CellType::SPACE) {
        if (camera_output == '^' || camera_output == '>'
            || camera_output == 'v' || camera_output == '<') {
            new_cell_type = CellType::SCAFFOLD;
            robo.facing = static_cast<Direction>(camera_output);
            robo.position = coord_type{row, col};
        } else {
            std::cerr << "Invalid camera output: " << camera_output;
            exit(5);
        }
    }
    auto new_cell = std::make_shared<Cell>(new_cell_type);
    grid[coord_type{row, col}] = new_cell;
    if (new_cell->type == CellType::SCAFFOLD) {
        if (row >= 1 && grid[coord_type{row-1, col}]->type == CellType::SCAFFOLD) {
            auto north_neighbor = grid[coord_type{row-1, col}];
            new_cell->north = north_neighbor;
            north_neighbor->south = new_cell;
        }
        if (col >= 1 && grid[coord_type{row, col-1}]->type == CellType::SCAFFOLD) {
            auto west_neighbor = grid[coord_type{row, col-1}];
            new_cell->west = west_neighbor;
            west_neighbor->east = new_cell;
        }
    }
}


void draw_grid(const grid_type &grid,
               const Robot &robo) {
    for (auto iter = grid.cbegin(); iter != grid.cend(); ++iter) {
        if (iter->first == robo.position) {
            std::cout << static_cast<char>(robo.facing);
        } else {
            std::cout << static_cast<char>(iter->second->type);
        }
        if (std::next(iter) != grid.cend()
            && std::next(iter)->first[0] > iter->first[0]) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}


int calculate_alignment_params(const grid_type &grid) {
    auto align_param = 0;
    for (auto &pair: grid) {
        auto cell = pair.second;
        // This only includes 4-way intersecions, not 3-way ones
        if (cell->north != nullptr && cell->east != nullptr
            && cell->south != nullptr && cell->west != nullptr) {
            align_param += pair.first[0] * pair.first[1];
        }
    }
    return align_param;
}


std::vector<Move> follow_scaffold(const grid_type &grid,
                                  Robot robo) {
    // Note: this assumes the robot isn't facing a dead end when it starts.
    std::vector<Move> moves;
    while (true) {
        auto cell = grid.at(robo.position);
        if (cell->type != CellType::SCAFFOLD) {
            std::cerr << "Robot is not on scaffold" << std::endl;
            exit(6);
        }
        auto forward_coords = get_coords_in_direction(robo.position, robo.facing);
        if (grid.find(forward_coords) != grid.end()
            && grid.at(forward_coords)->type == CellType::SCAFFOLD) {
            moves.push_back(Move::FORWARD);
            robo.position = forward_coords;
            continue;
        }

        auto right_coords = get_coords_in_direction(robo.position,
                                                    TURN_RIGHT.at(robo.facing));
        if (grid.find(right_coords) != grid.end()
            && grid.at(right_coords)->type == CellType::SCAFFOLD) {
            moves.push_back(Move::RIGHT);
            robo.facing = TURN_RIGHT.at(robo.facing);
            continue;
        }

        auto left_coords = get_coords_in_direction(robo.position,
                                                   TURN_LEFT.at(robo.facing));
        if (grid.find(left_coords) != grid.end()
            && grid.at(left_coords)->type == CellType::SCAFFOLD) {
            moves.push_back(Move::LEFT);
            robo.facing = TURN_LEFT.at(robo.facing);
            continue;
        }
        // Can't go forward, left or right, so robot is at a dead end.
        break;
    }
    return moves;
}


std::string compress(const std::vector<Move> &moves_vec,
                     size_t max_length = 20,
                     size_t num_movement_functions = 3) {
    // Convert Move::FORWARDs to numerals
    std::string moves_str;
    auto forward_count = 0;
    for (auto m: moves_vec) {
        if (m == Move::LEFT || m == Move::RIGHT) {
            if (forward_count > 0) {
                moves_str.append(std::to_string(forward_count));
                forward_count = 0;
            }
            moves_str.push_back(static_cast<char>(m));
        } else if (m == Move::FORWARD) {
            ++forward_count;
        }
    }
    if (forward_count > 0) {
        moves_str.append(std::to_string(forward_count));
    }
    std::cout << "Move string:" << std::endl;
    std::cout << moves_str << std::endl;

    // Compress the string into a main movement routine
    // and movement functions
    std::string function_names;
    for (size_t func = 0; func < num_movement_functions; ++func) {
        function_names.push_back(static_cast<char>(func) + 'A');
        std::cout << function_names[function_names.size() - 1] << std::endl;
        size_t best_compression = 0;
        // size_t best_start = 0;
        size_t best_len = 0;
        std::string best_str;
        for (size_t func_start = 0; func_start < moves_str.size(); ++func_start) {
            // std::cout << "func_start: " << func_start << std::endl;
            // Don't allow the move function to start mid-numeral
            if (func_start > 0
                && moves_str[func_start] != static_cast<char>(Move::LEFT)
                && moves_str[func_start] != static_cast<char>(Move::RIGHT)) {
                auto prev = moves_str[func_start - 1];
                if (prev != static_cast<char>(Move::LEFT)
                    && prev != static_cast<char>(Move::RIGHT)) {
                    continue;
                }
            }
            for (size_t func_len = 1; func_len <= max_length; ++func_len) {
                // std::cout << "func_len: " << func_len << std::endl;
                auto func_str = moves_str.substr(func_start, func_len);
                // Don't allow the move function to end mid-numeral
                auto func_end = func_start + func_len;
                if (func_end < moves_str.size()
                    && moves_str[func_end] != static_cast<char>(Move::LEFT)
                    && moves_str[func_end] != static_cast<char>(Move::RIGHT)) {
                    auto next = moves_str[func_end];
                    if (next != static_cast<char>(Move::LEFT)
                        && next != static_cast<char>(Move::RIGHT)) {
                        continue;
                    }
                }
                // Don't include other move functions
                if (func_str.find(function_names) != std::string::npos) {
                    break;
                }
                size_t compression = 0;
                // std::cout << "Checking for matches" << std::endl;
                for (auto find_start = moves_str.find(func_str);
                     find_start < moves_str.size();
                     find_start = moves_str.find(func_str, find_start + 1)) {
                    // std::cout << "Match at " << find_start << std::endl;
                    compression += func_len - 1;
                }
                if (compression > best_compression) {
                    best_compression = compression;
                    // best_start = func_start;
                    best_len = func_len;
                    best_str = func_str;
                }
            }
        }
        std::cout << "Best move function is: " << best_str << std::endl;

        // Replace instances of this move function with its letter
        std::cout << "Compressing" << std::endl;
        for (auto find_start = moves_str.find(best_str);
             find_start < moves_str.size();
             find_start = moves_str.find(best_str)) {
            moves_str.replace(find_start, best_len, 1, function_names[func]);
            // std::cout << moves_str << std::endl;
        }
    }
    std::cout << "Compressed string: " << moves_str << std::endl;
    return moves_str;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    grid_type grid;
    size_t row_index = 0;
    size_t column_index = 0;
    Robot robo;

    auto intcode_input = []() -> intcode_type {
        return 0;
    };

    auto intcode_output = [&](intcode_type output) -> void {
        auto output_char = static_cast<char>(output);
        if (output_char == '\n') {
            column_index = 0;
            ++row_index;
        } else {
            add_grid_cell(grid, robo, output_char, row_index, column_index);
            ++column_index;
        }
    };

    run_intcode_program(program, intcode_input, intcode_output);
    draw_grid(grid, robo);
    auto align_param = calculate_alignment_params(grid);
    auto moves = follow_scaffold(grid, robo);
    for (auto &move: moves) {
        std::cout << static_cast<char>(move);
    }
    std::cout << std::endl;
    compress(moves);

    std::cout << "PART 1" << std::endl;
    std::cout << "Alignment paramter: " << align_param << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
