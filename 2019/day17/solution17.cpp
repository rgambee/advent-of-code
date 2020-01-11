#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
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
constexpr size_t NUM_MOVEMENT_FUNCTIONS = 3;
constexpr size_t MAX_MOVEMENT_FUNCTION_LENGTH = 20;


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
            std::stringstream error_message;
            error_message << "Unknown direction: " << static_cast<char>(dir);
            throw std::invalid_argument(error_message.str());
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
            std::stringstream error_message;
            error_message << "Invalid camera output: " << camera_output;
            throw std::runtime_error(error_message.str());
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
            throw std::logic_error("Robot is not on scaffold");
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


std::string move_vector_to_string(std::vector<Move>::const_iterator start,
                                  std::vector<Move>::const_iterator stop) {
    // Convert Move::FORWARDs to numerals
    std::stringstream moves_stream;
    auto forward_count = 0;
    for (; start != stop; ++start) {
        if (*start == Move::LEFT || *start == Move::RIGHT) {
            if (forward_count > 0) {
                moves_stream << std::to_string(forward_count) << ',';
                forward_count = 0;
            }
            moves_stream << static_cast<char>(*start) << ',';
        } else if (*start == Move::FORWARD) {
            ++forward_count;
        }
    }
    if (forward_count > 0) {
        moves_stream << std::to_string(forward_count) << ',';
    }
    auto result = moves_stream.str();
    // Removing final trailing comma
    result.erase(result.size() - 1, 1);
    return result;
}


std::vector<std::vector<size_t> > valid_move_functions(
    const std::vector<Move> &moves_vec,
    const std::vector<size_t> &func_starts,
    const std::vector<size_t> &func_lengths,
    size_t &failed_func,
    size_t max_length = MAX_MOVEMENT_FUNCTION_LENGTH) {
    // A valid set of move functions satisfies all of the following:
    //      No overlapping
    //      None longer than max_length characters
    //      Fully describe the input move sequence
    //      Main movement routine is not longer than max_length function calls
    if (func_starts.size() != func_lengths.size()) {
        throw std::invalid_argument(
            "func_starts and func_lengths do not have the same size");
    }
    std::vector<size_t> func_ends(func_starts.size());
    std::vector<std::string> func_strings(func_starts.size());
    for (size_t i = 0; i < func_starts.size(); ++i) {
        func_ends[i] = func_starts[i] + func_lengths[i];
        if (func_ends[i] > moves_vec.size()) {
            failed_func = i;
            return {};
        }
        for (size_t j = 0; j < i; ++j) {
            // Simple but incomplete overlap check
            if ((func_starts[j] <= func_starts[i] && func_starts[i] < func_ends[j])
                || (func_starts[j] < func_ends[i] && func_ends[i] <= func_ends[j])) {
                failed_func = i;
                return {};
            }
        }
        func_strings[i] = move_vector_to_string(
            std::next(moves_vec.cbegin(), func_starts[i]),
            std::next(moves_vec.cbegin(), func_ends[i]));
        if (func_strings[i].size() > max_length) {
            // Move function is too long
            failed_func = i;
            return {};
        }
    }
    std::vector<std::vector<size_t> > func_matches(func_starts.size());
    for (size_t i = 0; i < func_starts.size(); ++i) {
        auto func_begin = std::next(moves_vec.cbegin(), func_starts[i]);
        auto func_end = std::next(moves_vec.cbegin(), func_ends[i]);
        auto match_iter = std::search(moves_vec.cbegin(), moves_vec.cend(),
                                      func_begin, func_end);
        for (; match_iter != moves_vec.cend();
             match_iter = std::search(std::next(match_iter, func_lengths[i]),
                                      moves_vec.cend(),
                                      func_begin, func_end)) {
            size_t match_index = std::distance(moves_vec.cbegin(), match_iter);
            auto match_end = match_index + func_lengths[i];
            auto add_match = true;
            std::vector<std::vector<size_t> > matches_to_remove(func_starts.size());
            // Comprehensive overlap check
            for (size_t j = 0; j < i; ++j) {
                for (size_t k = 0; k < func_matches[j].size(); ++k) {
                    auto other_match = func_matches[j][k];
                    auto other_end = other_match + func_lengths[j];
                    auto match_starts_within_other = (other_match <= match_index
                                                      && match_index < other_end);
                    auto match_ends_within_other = (other_match < match_end
                                                    && match_end <= other_end);
                    auto other_starts_within_match = (match_index <= other_match
                                                      && other_match < match_end);
                    auto other_ends_within_match = (match_index < other_end
                                                    && other_end <= match_end);
                    if (match_starts_within_other && match_ends_within_other) {
                        // B is contained within A:
                        //      AAAAAA
                        //        BB
                        // This isn't necessarily a problem,
                        // but this shouldn't be counted as a match for B.
                        add_match = false;
                    } else if (other_starts_within_match && other_ends_within_match) {
                        // A is contained within B:
                        //        AA
                        //      BBBBBB
                        // This isn't necessarily a problem,
                        // but A's match needs to be replaced with B's.
                        matches_to_remove[j].push_back(k);
                    } else if (match_starts_within_other || match_ends_within_other) {
                        // This is a partial overlap:
                        //      AAAAAA      OR      AAAAAA
                        //         BBBBBB        BBBBBB
                        // Mark i as the failed function
                        // since making it longer won't resolve this.
                        failed_func = i;
                        return {};
                    }
                }
            }
            if (add_match) {
                func_matches[i].push_back(match_index);
                for (size_t j = 0; j < matches_to_remove.size(); ++j) {
                    for (int k = static_cast<int>(matches_to_remove[j].size()) - 1;
                         k >= 0; --k) {
                        func_matches[j].erase(std::next(func_matches[j].begin(),
                                                        matches_to_remove[j][k]));
                    }
                }
            }
        }
    }

    size_t combined_move_func_length = 0;
    size_t num_func_calls = 0;
    for (size_t i = 0; i < func_starts.size(); ++i) {
        combined_move_func_length += func_lengths[i] * func_matches[i].size();
        num_func_calls += func_matches[i].size();
    }
    // We've already checked for overlap, so to check whether the entire
    // move sequence is covered, we can compare its length to the combined
    // lengths of the movement functions.
    if (combined_move_func_length != moves_vec.size()) {
        return {};
    }
    // Check whether the main movement routine contains too many function calls
    if (num_func_calls > max_length) {
        return {};
    }
    return func_matches;
}


std::string create_main_movement_routine(
            const std::vector<std::vector<size_t> > func_matches) {
    auto index_to_char = [](size_t index) -> char {
        return static_cast<char>(index) + 'A';
    };

    std::stringstream mmr_stream;
    std::vector<size_t> curr_indices(func_matches.size(), 0);
    while (true) {
        auto func_to_add = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < func_matches.size(); ++i) {
            if (curr_indices[i] < func_matches[i].size()) {
                if (func_to_add >= func_matches.size()
                    || func_matches[func_to_add][curr_indices[func_to_add]]
                       > func_matches[i][curr_indices[i]]) {
                    func_to_add = i;
                }
            }
        }
        if (func_to_add > func_matches.size()) {
            break;
        }
        mmr_stream << index_to_char(func_to_add) << ',';
        ++curr_indices[func_to_add];
    }
    auto result = mmr_stream.str();
    // Removing final trailing comma
    result.erase(result.size() - 1, 1);
    return result;
}


std::vector<std::string> compress(const std::vector<Move> &moves_vec) {
    std::vector<size_t> func_starts(NUM_MOVEMENT_FUNCTIONS, 0);
    std::vector<size_t> func_lengths(NUM_MOVEMENT_FUNCTIONS, 1);
    std::vector<std::string> result;
    while (true) {
        size_t failed_func = NUM_MOVEMENT_FUNCTIONS + 1;
        for (size_t i = 0; i < func_starts.size(); ++i) {
            std::cout << func_starts[i] << ":" << func_lengths[i] << ", ";
        }
        std::cout << '\r';
        auto func_matches = valid_move_functions(moves_vec, func_starts,
                                                 func_lengths, failed_func);
        if (func_matches.size() == func_starts.size()) {
            std::cout << "Solution found!" << std::endl;
            result.push_back(create_main_movement_routine(func_matches));
            for (size_t i = 0; i < func_starts.size(); ++i) {
                result.push_back(move_vector_to_string(
                    std::next(moves_vec.cbegin(), func_starts[i]),
                    std::next(moves_vec.cbegin(), func_starts[i] + func_lengths[i])));
            }
            return result;
        }
        // for (size_t i = 0; i < func_starts.size(); ++i) {
        //     std::cout << func_starts[i] << " " << func_lengths[i] << std::endl;
        // }
        // std::cout << "Failed " << failed_func << std::endl;
        for (size_t i = func_starts.size() - 1; true; --i) {
            if (i != failed_func && func_starts[i] + func_lengths[i] < moves_vec.size()) {
                ++func_lengths[i];
                for (size_t j = i+1; j < func_starts.size(); ++j) {
                    func_starts[j] = func_starts[i] + func_lengths[i];
                    func_lengths[j] = 1;
                }
                break;
            }
            if (func_starts[i] < moves_vec.size() - 1) {
                ++func_starts[i];
                func_lengths[i] = 1;
                for (size_t j = i+1; j < func_starts.size(); ++j) {
                    func_starts[j] = func_starts[i] + func_lengths[i];
                    func_lengths[j] = 1;
                }
                break;
            }
            if (i == 0) {
                std::cout << "No solution possible" << std::endl;
                return result;
            }
        }
    }
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    grid_type grid;
    size_t row_index = 0;
    size_t column_index = 0;
    Robot robo;

    auto part1_input = []() -> intcode_type {
        return 0;
    };

    auto part1_output = [&](intcode_type output) -> void {
        auto output_char = static_cast<char>(output);
        if (output_char == '\n') {
            column_index = 0;
            ++row_index;
        } else {
            add_grid_cell(grid, robo, output_char, row_index, column_index);
            ++column_index;
        }
    };

    // Read grid and calculate alignment parameters
    run_intcode_program(program, part1_input, part1_output);
    draw_grid(grid, robo);
    auto align_param = calculate_alignment_params(grid);

    // Create path that follows scaffold and compress it
    // into a main movement routine and several movement functions
    auto moves = follow_scaffold(grid, robo);
    auto movement_routines = compress(moves);
    for (auto &s: movement_routines) {
        std::cout << s << std::endl;
    }

    // Send robot movement instructions
    std::stringstream robot_input;
    for (size_t i = 0; i < movement_routines.size(); ++i) {
        robot_input << movement_routines[i] << '\n';
    }
    // Don't show video feed
    robot_input << "n\n";
    auto part2_input = [&robot_input]() -> intcode_type {
        return static_cast<intcode_type>(robot_input.get());
    };

    intcode_type dust_collected;
    auto part2_output = [&dust_collected](intcode_type output) -> void {
        dust_collected = output;
    };
    // Switch program mode
    program[0] = 2;
    run_intcode_program(program, part2_input, part2_output);

    std::cout << "PART 1" << std::endl;
    std::cout << "Alignment paramter: " << align_param << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Dust collected: " << dust_collected << std::endl;
    return 0;
}
