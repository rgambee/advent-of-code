#include <array>
#include <iostream>
#include <map>

#include "intcode.h"
#include "utils.h"


enum class Tile {
    EMPTY = 0,
    WALL = 1,
    BLOCK = 2,
    PADDLE = 3,
    BALL = 4
};

enum class Joystick {
    NEUTRAL = 0,
    LEFT = -1,
    RIGHT = 1
};

using coord_type = std::array<int, 2>;
using tiles_type = std::map<coord_type, Tile>;


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    tiles_type tiles;
    auto intcode_input = [&tiles]() -> intcode_type {
        coord_type ball_coords;
        coord_type paddle_coords;
        for (auto iter = tiles.begin(); iter != tiles.end(); ++iter) {
            if (iter->second == Tile::BALL) {
                ball_coords = iter->first;
            } else if (iter->second == Tile::PADDLE) {
                paddle_coords = iter->first;
            }
        }
        auto input = static_cast<intcode_type>(Joystick::NEUTRAL);
        if (paddle_coords[0] < ball_coords[0]) {
            input = static_cast<intcode_type>(Joystick::RIGHT);
        } else if (paddle_coords[0] > ball_coords[0]) {
            input = static_cast<intcode_type>(Joystick::LEFT);
        }
        return input;
    };

    coord_type active;
    auto output_index = 0;
    auto score = 0;
    auto update_score = false;
    auto intcode_output = [&](intcode_type output) -> void {
        switch (output_index) {
            case 0:
                active[0] = output;
                update_score = output == -1;
                break;
            case 1:
                active[1] = output;
                update_score = update_score && output == 0;
                break;
            case 2:
                if (update_score) {
                    score = output;
                } else {
                    tiles[active] = static_cast<Tile>(output);
                }
                break;
            default:
                std::cerr << "Invalid output index: " << output_index << std::endl;
                exit(3);
        }
        output_index = (output_index + 1) % 3;
    };
    run_intcode_program(program, intcode_input, intcode_output);

    auto num_blocks = 0;
    for (auto iter = tiles.begin(); iter != tiles.end(); ++iter) {
        if (iter->second == Tile::BLOCK) {
            ++num_blocks;
        }
    }

    // Set address 0 to 2 for free play
    program[0] = 2;
    run_intcode_program(program, intcode_input, intcode_output);

    std::cout << "PART 1" << std::endl;
    std::cout << "Number of blocks: " << num_blocks << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Final score: " << score << std::endl;
    return 0;
}
