#include <cmath>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils.h"

using chem_type = std::string;
using quantity_type = std::pair<chem_type, int>;
using inventory_type = std::unordered_map<chem_type, long long>;
using reaction_type = std::pair<inventory_type, quantity_type>;
const std::regex LINE_REGEX("([[:alnum:], ]+) => ([[:alnum:] ]+)");
const std::regex QUANT_REGEX("([[:digit:]]+) ([[:alnum:]]+)");
const chem_type ORE_NAME{"ORE"};
const chem_type FUEL_NAME{"FUEL"};
constexpr long long STARTING_ORE = 1000000000000;


quantity_type string_to_quantity(const std::string &input_str) {
    std::smatch match;
    if (!std::regex_match(input_str, match, QUANT_REGEX)) {
        std::stringstream error_message;
        error_message << "No match for string: " << input_str;
        throw std::invalid_argument(error_message.str());
    }
    auto num = std::stoi(match.str(1));
    chem_type chem = match.str(2);
    return quantity_type{chem, num};
}


void print_reaction(const reaction_type &react) {
    for (auto &r: react.first) {
        std::cout << r.second << " " << r.first << " ";
    }
    std::cout << "=> " << react.second.second << " " << react.second.first << std::endl;
}


void make_fuel(const std::unordered_map<chem_type, reaction_type> &reactions,
               inventory_type &inventory) {
    reaction_type final_reaction = reaction_type(reactions.at(FUEL_NAME));
    // print_reaction(final_reaction);
    while (final_reaction.first.size() > 1) {
        for (auto iter = final_reaction.first.begin();
             iter != final_reaction.first.end(); ++iter) {
            auto to_replace = *iter;
            if (to_replace.first == ORE_NAME) {
                continue;
            }
            // Replace reactant with the reaction that produces it
            final_reaction.first.erase(iter);
            auto producing_reaction = reactions.at(to_replace.first);
            // How many copies of this reaction do we need to add?
            auto num_required = to_replace.second;
            auto inventory_to_use = std::min(num_required,
                                             inventory[to_replace.first]);
            num_required -= inventory_to_use;
            inventory[to_replace.first] -= inventory_to_use;
            if (num_required > 0) {
                auto n = static_cast<int>(std::ceil(
                    static_cast<float>(num_required)
                    / static_cast<float>(producing_reaction.second.second)));
                if (n == 0) {
                    throw std::logic_error("Replacing with zero reactions");
                }
                for (auto &quant: producing_reaction.first) {
                    final_reaction.first[quant.first] += n * quant.second;
                }
                inventory[to_replace.first] += (
                    producing_reaction.second.second * n - num_required);
            }
            // print_reaction(final_reaction);
            break;
        }
    }
    inventory[ORE_NAME] -= final_reaction.first[ORE_NAME];
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    std::string line;
    std::unordered_map<chem_type, reaction_type> reactions;
    while (std::getline(input_stream, line)) {
        std::smatch match;
        if (std::regex_match(line, match, LINE_REGEX)) {
            auto reactants_str = match.str(1);
            auto products_str = match.str(2);

            inventory_type reactants;
            std::regex_iterator<std::string::iterator> riter{
                reactants_str.begin(), reactants_str.end(), QUANT_REGEX
            };
            for (; riter != std::regex_iterator<std::string::iterator>{}; ++riter) {
                auto quant = string_to_quantity(riter->str());
                reactants[quant.first] = quant.second;
            }

            auto products = string_to_quantity(products_str);
            auto result = reactions.emplace(
                products.first, reaction_type{reactants, products});
            if (!result.second) {
                std::stringstream error_message;
                error_message << "Multiple reactions produce " << products.first;
                throw std::runtime_error(error_message.str());
            }
        } else {
            std::cerr << "Could not parse line: " << line << std::endl;
        }
    }

    inventory_type inventory;
    inventory[ORE_NAME] = STARTING_ORE;
    long long num_ore_for_first_fuel = -1;
    auto num_fuel_produced = 0;
    while (inventory[ORE_NAME] > 0) {
        make_fuel(reactions, inventory);
        if (inventory[ORE_NAME] >= 0) {
            ++num_fuel_produced;
            std::cout << "Remaining ore: " << inventory[ORE_NAME] << "\r";
        }
        if (num_ore_for_first_fuel < 0) {
            num_ore_for_first_fuel = STARTING_ORE - inventory[ORE_NAME];
        }
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Number of ore required for first fuel: " << num_ore_for_first_fuel << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Total number of fuel produced: " << num_fuel_produced << std::endl;
    return 0;
}
