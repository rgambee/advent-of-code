#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"


using asteroids_row_type = std::vector<bool>;
using asteroids_type = std::vector<asteroids_row_type>;
using coord_type = std::array<asteroids_type::size_type, 2>;


int gcd(int a, int b) {
    // Euclidean method: https://en.wikipedia.org/wiki/Euclidean_algorithm
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}


bool is_visible(asteroids_type asteroids,
                coord_type source,
                coord_type target) {
    // Is the asteroid at target visible from source?
    auto rise = static_cast<int>(target[1]) - static_cast<int>(source[1]);
    auto run = static_cast<int>(target[0]) - static_cast<int>(source[0]);
    auto norm = static_cast<int>(std::abs(gcd(rise, run)));
    rise /= norm;
    run /= norm;
    if (rise == 0) {
        run = run > 0 ? 1 : -1;
    }
    if (run == 0) {
        rise = rise > 0 ? 1 : -1;
    }
    coord_type current{source[0] + run, source[1] + rise};
    while (current[0] != target[0] || current[1] != target[1]) {
        // std::cout << "        " << current[0] << "," << current[1] << std::endl;
        if (asteroids[current[0]][current[1]]) {
            return false;
        }
        current[0] += run;
        current[1] += rise;
    }
    return true;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    asteroids_type asteroids;
    std::string line;
    while (std::getline(input_stream, line)) {
        asteroids_row_type row;
        for (auto c: line) {
            if (c == '#') {
                row.push_back(true);
            } else if (c == '.') {
                row.push_back(false);
            } else {
                std::cerr << "Unknown input character " << c << std::endl;
                exit(3);
            }
        }
        asteroids.push_back(row);
    }

    auto most_visible = 0;
    for (asteroids_type::size_type source_x = 0; source_x < asteroids.size(); ++source_x) {
        for (asteroids_row_type::size_type source_y = 0; source_y < asteroids[source_x].size(); ++source_y) {
            // std::cout << source_x << "," << source_y << std::endl;
            if (!asteroids[source_x][source_y]) {
                continue;
            }
            coord_type source{source_x, source_y};
            auto visible = 0;
            for (asteroids_type::size_type target_x = 0; target_x < asteroids.size(); ++ target_x) {
                for (asteroids_row_type::size_type target_y = 0; target_y < asteroids[target_x].size(); ++target_y) {
                    if (!asteroids[target_x][target_y]) {
                        continue;
                    }
                    if (source_x == target_x && source_y == target_y) {
                        // An asteroid is not considered visible from itself
                        continue;
                    }
                    coord_type target{target_x, target_y};
                    // std::cout << "    " << target_x << "," << target_y << std::endl;
                    if (is_visible(asteroids, source, target)) {
                        ++visible;
                    }
                }
            }
            most_visible = std::max(most_visible, visible);
        }
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Most asteroids visible from one location: " << most_visible << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
