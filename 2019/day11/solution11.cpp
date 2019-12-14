#include <array>
#include <iostream>
#include <map>
#include <set>

#include "intcode.h"
#include "utils.h"


enum class Paint {
    BLACK = 0,
    WHITE = 1
};

enum class Direction {
    LEFT = 0,
    RIGHT = 1,
    UP,
    DOWN
};

using coord_type = std::array<int, 2>;
using panels_type = std::map<coord_type, Paint>;


class Robot {
public:
    Robot() = default;

    coord_type step() {
        switch (facing) {
            case Direction::UP:
                location = coord_type{location[0], location[1] + 1};
                break;
            case Direction::DOWN:
                location = coord_type{location[0], location[1] - 1};
                break;
            case Direction::LEFT:
                location = coord_type{location[0] - 1, location[1]};
                break;
            case Direction::RIGHT:
                location = coord_type{location[0] + 1, location[1]};
                break;
        }
        return location;
    }

    Direction turn(Direction relative_dir) {
        switch (relative_dir) {
            case Direction::LEFT:
                switch (facing) {
                    case Direction::UP:
                        facing = Direction::LEFT;
                        break;
                    case Direction::RIGHT:
                        facing = Direction::UP;
                        break;
                    case Direction::DOWN:
                        facing = Direction::RIGHT;
                        break;
                    case Direction::LEFT:
                        facing = Direction::DOWN;
                        break;
                }
                break;
            case Direction::RIGHT:
                switch (facing) {
                    case Direction::UP:
                        facing = Direction::RIGHT;
                        break;
                    case Direction::RIGHT:
                        facing = Direction::DOWN;
                        break;
                    case Direction::DOWN:
                        facing = Direction::LEFT;
                        break;
                    case Direction::LEFT:
                        facing = Direction::UP;
                        break;
                }
                break;
            default:
                std::cerr << "Relative direction must be LEFT or RIGHT, not ";
                std::cerr << int(relative_dir) << std::endl;
                exit(3);
        }
        return facing;
    }

    void paint(Paint color, panels_type &panels) {
        panels[location] = color;
    }

    Paint get_paint_color(panels_type &panels) {
        return panels[location];
    }

    coord_type location{0, 0};
    Direction facing = Direction::UP;
};


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    panels_type panels;
    Robot robo;
    bool toggle = true;
    std::set<coord_type> painted;

    auto intcode_input = [&robo, &panels,&toggle]() -> intcode_type {
        auto inp = static_cast<intcode_type>(robo.get_paint_color(panels));
        return inp;
    };
    auto intcode_output = [&robo, &panels, &toggle, &painted](intcode_type output) -> void {
        if (toggle) {
            // First of output pair is the color to paint the current panel
            robo.paint(static_cast<Paint>(output), panels);
            painted.insert(robo.location);
        } else {
            // Second of output pair is the direction to turn
            robo.turn(static_cast<Direction>(output));
            robo.step();
        }
        toggle = !toggle;
    };
    run_intcode_program(program, intcode_input, intcode_output);

    std::cout << "PART 1" << std::endl;
    std::cout << "Number of panels painted: " << painted.size() << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
