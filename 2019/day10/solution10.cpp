#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include "utils.h"


using asteroids_row_type = std::vector<bool>;
using asteroids_type = std::vector<asteroids_row_type>;
using coord_type = std::array<asteroids_type::size_type, 2>;
const auto NOT_FOUND = std::numeric_limits<std::tuple_element<0, coord_type>::type>::max();


int gcd(int a, int b) {
    // Euclidean method: https://en.wikipedia.org/wiki/Euclidean_algorithm
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}


coord_type find_asteroid_along_line(const asteroids_type &asteroids,
                                    const coord_type &source,
                                    int rise, int run) {
    auto norm = static_cast<int>(std::abs(gcd(rise, run)));
    rise /= norm;
    run /= norm;
    if (rise == 0) {
        run = run > 0 ? 1 : -1;
    }
    if (run == 0) {
        rise = rise > 0 ? 1 : -1;
    }
    coord_type current{source[0] + rise, source[1] + run};
    while (current[0] < asteroids.size()
           && current[1] < asteroids[current[0]].size()) {
        if (asteroids[current[0]][current[1]]) {
            return current;
        }
        current[0] += rise;
        current[1] += run;
    }
    return coord_type{NOT_FOUND, NOT_FOUND};
}


bool is_visible(const asteroids_type &asteroids,
                const coord_type &source,
                const coord_type &target) {
    // Is the asteroid at target visible from source?
    auto rise = static_cast<int>(target[0]) - static_cast<int>(source[0]);
    auto run = static_cast<int>(target[1]) - static_cast<int>(source[1]);
    auto found = find_asteroid_along_line(asteroids, source, rise, run);
    return found == target;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    asteroids_type asteroids;
    std::string line;
    asteroids_type::size_type max_size = 0;
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
        max_size = std::max(max_size, row.size());
    }
    max_size = std::max(max_size, asteroids.size());

    auto most_visible = 0;
    coord_type station;
    for (asteroids_type::size_type source_y = 0; source_y < asteroids.size(); ++source_y) {
        for (asteroids_row_type::size_type source_x = 0; source_x < asteroids[source_y].size(); ++source_x) {
            if (!asteroids[source_y][source_x]) {
                continue;
            }
            coord_type source{source_y, source_x};
            auto visible = 0;
            for (asteroids_type::size_type target_y = 0; target_y < asteroids.size(); ++ target_y) {
                for (asteroids_row_type::size_type target_x = 0; target_x < asteroids[target_y].size(); ++target_x) {
                    if (!asteroids[target_y][target_x]) {
                        continue;
                    }
                    if (source_x == target_x && source_y == target_y) {
                        // An asteroid is not considered visible from itself
                        continue;
                    }
                    coord_type target{target_y, target_x};
                    if (is_visible(asteroids, source, target)) {
                        ++visible;
                    }
                }
            }
            if (visible > most_visible) {
                most_visible = std::max(most_visible, visible);
                station = source;
            }
        }
    }
    // std::cout << "Station at (" << station[0] << ", " << station[1] << ")" << std::endl;

    auto compare = [](std::array<int, 2> arr1, std::array<int, 2> arr2) -> bool {
        // Denominator may be 0, which is fine
        auto ratio1 = static_cast<double>(arr1[0]) / static_cast<double>(arr1[1]);
        auto ratio2 = static_cast<double>(arr2[0]) / static_cast<double>(arr2[1]);
        return ratio1 < ratio2;
    };
    std::set<std::array<int, 2>, decltype(compare)> slope_set(compare);
    // First coordinate goes negative since the laser starts pointing up,
    // which is in the negative y direction
    for (int i = 0; i > -static_cast<int>(max_size); --i) {
        for (int j = 0; j < static_cast<int>(max_size); ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            int i_norm, j_norm;
            // Normalize so we don't get [1, 1] and [2, 2], etc.
            if (i == 0) {
                i_norm = 0;
                j_norm = j > 0 ? 1 : -1;
            } else if (j == 0) {
                i_norm = i > 0 ? 1 : -1;
                j_norm = 0;
            } else {
                auto norm = static_cast<int>(std::abs(gcd(i, j)));
                i_norm = i / norm;
                j_norm = j / norm;
            }
            // std::cout << "Adding " << i_norm << " / " << j_norm << std::endl;
            // Why doesnt slope_set.emplace({i_norm, j_norm}); work?
            std::array<int, 2> arr{i_norm, j_norm};
            slope_set.insert(arr);
        }
    }
    std::vector<std::array<int, 2> > slopes(slope_set.begin(), slope_set.end());
    slopes.reserve(4 * slopes.size());

    // slopes now covers a quarter circle
    // Append a reversed copy with the first indices negated
    for (auto iter = slopes.rbegin() + 1; iter != slopes.rend(); ++iter) {
        std::array<int, 2> arr{-(*iter)[0], (*iter)[1]};
        slopes.push_back(arr);
    }
    // slopes now covers a half circle
    // Append a reversed copy with the second indices negated
    for (auto iter = slopes.rbegin() + 1; iter != slopes.rend() - 1; ++iter) {
        std::array<int, 2> arr{(*iter)[0], -(*iter)[1]};
        slopes.push_back(arr);
    }

    coord_type asteroid_to_remove;
    auto num_removed = 0;
    for (int i = 0; num_removed < 200; ++i) {
        auto slope = slopes[i % slopes.size()];
        auto rise = slope[0];
        auto run = slope[1];
        // std::cout << "Rise: " << rise << ", run: " << run << std::endl;
        asteroid_to_remove = find_asteroid_along_line(asteroids, station, rise, run);
        if (asteroid_to_remove != coord_type{NOT_FOUND, NOT_FOUND}) {
            // std::cout << "To remove: (" << asteroid_to_remove[0] << ", ";
            // std::cout << asteroid_to_remove[1] << ")" << std::endl;
            if (!asteroids[asteroid_to_remove[0]][asteroid_to_remove[1]]) {
                std::cerr << "Asertoid not present" << std::endl;
                exit(4);
            }
            asteroids[asteroid_to_remove[0]][asteroid_to_remove[1]] = false;
            ++num_removed;
        }
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Most asteroids visible from one location: " << most_visible << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Result from 200th asteroid vaporized: ";
    // Swap coordinates here since the 0th element
    std::cout << 100 * asteroid_to_remove[1] + asteroid_to_remove[0] << std::endl;
    return 0;
}
