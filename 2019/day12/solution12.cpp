#include <iostream>
#include <regex>
#include <string>
#include <valarray>
#include <vector>

#include "utils.h"


constexpr auto NUM_DIMENSIONS = 3;
constexpr auto NUM_STEPS = 1000;
using coord_type = std::valarray<int>;
// Example line: <x=14, y=2, z=8>
const std::regex COORD_REGEX("<x=(-?\\d+), y=(-?\\d+), z=(-?\\d+)>");


class Moon {
public:
    Moon(const coord_type &pos): position(pos) {}

    void update_velocity(const Moon &other) {
        for (auto i = 0; i < NUM_DIMENSIONS; ++i) {
            auto difference = other.position[i] - position[i];
            if (difference > 0) {
                ++velocity[i];
            } else if (difference < 0) {
                --velocity[i];
            }
        }
    }

    void update_position() {
        position += velocity;
    }

    int kinetic_energy() const {
        return std::abs(velocity).sum();
    }

    int potential_energy() const {
        return std::abs(position).sum();
    }

    int total_energy() const {
        return kinetic_energy() * potential_energy();
    }

    coord_type position;
    coord_type velocity{0, 0, 0};
};


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    std::vector<Moon> moons;
    std::string line;
    while (std::getline(input_stream, line)) {
        std::smatch match;
        if (std::regex_match(line, match, COORD_REGEX)) {
            coord_type pos{std::stoi(match.str(1)),
                           std::stoi(match.str(2)),
                           std::stoi(match.str(3))};
            moons.emplace_back(pos);
        }
    }

    for (auto step = 0; step < NUM_STEPS; ++step) {
        // Update velocities
        for (auto iter1 = moons.begin(); iter1 != moons.end(); ++iter1) {
            for (auto iter2 = moons.begin(); iter2 != moons.end(); ++iter2) {
                if (iter1 == iter2) {
                    continue;
                }
                iter1->update_velocity(*iter2);
            }
        }
        // Update positions
        for (auto &moon: moons) {
            moon.update_position();
        }
    }

    // Calculate total energy
    auto energy = 0;
    for (auto &moon: moons) {
        energy += moon.total_energy();
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Total energy: " << energy << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
