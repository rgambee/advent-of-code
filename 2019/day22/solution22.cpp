#include <iostream>
#include <iterator>
#include <list>
#include <regex>
#include <string>

#include "utils.h"


constexpr size_t NUM_CARDS = 10007;
constexpr size_t DESIRED_CARD = 2019;
using deck_type = std::list<size_t>;
const std::regex DEAL_STACK_REGEX("deal into new stack");
const std::regex DEAL_INCREMENT_REGEX("deal with increment ([[:digit:]]+)");
const std::regex CUT_REGEX("cut (-?[[:digit:]]+)");


void deal_into_new_stack(deck_type &deck) {
    deck.reverse();
}


void deal_with_increment(deck_type &deck, size_t increment) {
    deck_type copy(deck);
    auto dest = deck.begin();
    for (auto card = copy.begin(); card != copy.end(); ++card) {
        *dest = *card;
        for (size_t i = 0; i < increment; ++i) {
            std::advance(dest, 1);
            if (dest == deck.end()) {
                dest = deck.begin();
            }
        }
    }
}


void cut(deck_type &deck, int cut_pos) {
    if (cut_pos > 0) {
        deck.splice(deck.end(), deck,
                    deck.begin(), std::next(deck.begin(), cut_pos));
    } else if (cut_pos < 0) {
        deck.splice(deck.begin(), deck,
                    std::prev(deck.end(), -cut_pos), deck.end());
    }
}


void print_deck(const deck_type &deck) {
    for (auto &item: deck) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);

    deck_type deck;
    for (size_t i = 0; i < NUM_CARDS; ++i) {
        deck.push_back(i);
    }

    std::string line;
    while (std::getline(input_stream, line)) {
        std::smatch match;
        if (std::regex_match(line, match, DEAL_STACK_REGEX)) {
            deal_into_new_stack(deck);
        } else if (std::regex_match(line, match, DEAL_INCREMENT_REGEX)) {
            deal_with_increment(deck, std::stoi(match.str(1)));
        } else if (std::regex_match(line, match, CUT_REGEX)) {
            cut(deck, std::stoi(match.str(1)));
        } else {
            std::cerr << "Invalid line: " << line << std::endl;
            exit(3);
        }
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "Position of card " << DESIRED_CARD << ": ";
    for (auto iter = deck.begin(); iter != deck.end(); ++iter) {
        if (*iter == DESIRED_CARD) {
            std::cout << std::distance(deck.begin(), iter) << std::endl;
            break;
        }
    }
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
