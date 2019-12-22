#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <set>
#include <valarray>
#include <vector>

#include "utils.h"


constexpr auto NUM_DIMENSIONS = 3;
constexpr auto NUM_STEPS = 1000;
using coord_type = std::valarray<int>;
using state_type = std::vector<int>;
// Example line: <x=14, y=2, z=8>
const std::regex COORD_REGEX("<x=(-?\\d+), y=(-?\\d+), z=(-?\\d+)>");


long long gcd(long long a, long long b) {
    // Euclidean method: https://en.wikipedia.org/wiki/Euclidean_algorithm
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}


long long lcm(long long a, long long b) {
    // https://en.wikipedia.org/wiki/Least_common_multiple
    return (a / gcd(a, b)) * b;
}


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

    std::map<state_type, long long> states_x;
    std::map<state_type, long long> states_y;
    std::map<state_type, long long> states_z;
    long long period_x = -1, period_y = -1, period_z = -1;

    auto energy_after_n_steps = 0;
    for (long long step = 0; period_x < 0 || period_y < 0 || period_z < 0; ++step) {
        state_type new_state_x, new_state_y, new_state_z;
        // Update velocities
        for (auto iter1 = moons.begin(); iter1 != moons.end(); ++iter1) {
            for (auto iter2 = moons.begin(); iter2 != moons.end(); ++iter2) {
                if (iter1 == iter2) {
                    continue;
                }
                iter1->update_velocity(*iter2);
                new_state_x.push_back(iter1->velocity[0]);
                new_state_y.push_back(iter1->velocity[1]);
                new_state_z.push_back(iter1->velocity[2]);
            }
        }
        // Update positions
        for (auto &moon: moons) {
            moon.update_position();
            new_state_x.push_back(moon.position[0]);
            new_state_y.push_back(moon.position[1]);
            new_state_z.push_back(moon.position[2]);
        }
        auto past_x_state = states_x.find(new_state_x);
        if (past_x_state == states_x.end()) {
            states_x[new_state_x] = step;
        } else if (period_x < 0) {
            period_x = step - past_x_state->second;
            std::cout << "Period x = " << period_x << std::endl;
        }
        auto past_y_state = states_y.find(new_state_y);
        if (past_y_state == states_y.end()) {
            states_y[new_state_y] = step;
        } else if (period_y < 0) {
            period_y = step - past_y_state->second;
            std::cout << "Period y = " << period_y << std::endl;
        }
        auto past_z_state = states_z.find(new_state_z);
        if (past_z_state == states_z.end()) {
            states_z[new_state_z] = step;
        } else if (period_z < 0) {
            period_z = step - past_z_state->second;
            std::cout << "Period z = " << period_z << std::endl;
        }

        if (step == NUM_STEPS - 1) {
            // Calculate total energy
            for (auto &moon: moons) {
                energy_after_n_steps += moon.total_energy();
            }
        }
    }
    auto total_period = lcm(lcm(period_x, period_y), period_z);

    std::cout << std::endl << "PART 1" << std::endl;
    std::cout << "Total energy: " << energy_after_n_steps << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Period of system: " << total_period << std::endl;
    return 0;
}
