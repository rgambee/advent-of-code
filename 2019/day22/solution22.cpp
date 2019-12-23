#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "utils.h"


constexpr long long PART1_DECK_SIZE = 10007;
constexpr long long PART1_DESIRED_CARD = 2019;
constexpr long long PART2_DECK_SIZE = 119315717514047;
constexpr long long PART2_REPEATS = 101741582076661;
constexpr long long PART2_DESIRED_CARD = 2020;
const std::regex DEAL_STACK_REGEX("deal into new stack");
const std::regex DEAL_INCREMENT_REGEX("deal with increment ([[:digit:]]+)");
const std::regex CUT_REGEX("cut (-?[[:digit:]]+)");
using deck_type = std::list<long long>;


class Instruction {
public:
    virtual ~Instruction() {};
    virtual void simulate_slow(deck_type &deck) const = 0;
    virtual long long simulate_fast(long long deck_size,
                                    long long starting_pos) const = 0;
};


class DealIntoNewStack: public Instruction {
public:
    void simulate_slow(deck_type &deck) const override {
        deck.reverse();
    }

    long long simulate_fast(long long deck_size,
                            long long starting_pos) const override {
        return deck_size - starting_pos - 1;
    }
};


class DealWithIncrement: public Instruction {
public:
    DealWithIncrement(long long inc): increment(inc) {}

    void simulate_slow(deck_type &deck) const override {
        deck_type copy(deck);
        auto dest = deck.begin();
        for (auto card = copy.begin(); card != copy.end(); ++card) {
            *dest = *card;
            for (long long i = 0; i < increment; ++i) {
                std::advance(dest, 1);
                if (dest == deck.end()) {
                    dest = deck.begin();
                }
            }
        }
    }

    long long simulate_fast(long long deck_size,
                            long long starting_pos) const override {
        return ((starting_pos % deck_size) * (increment % deck_size)) % deck_size;
    }

    long long increment = 1;
};


class Cut: public Instruction {
public:
    Cut(long long pos): position(pos) {}

    void simulate_slow(deck_type &deck) const override {
        if (position > 0) {
            deck.splice(deck.end(), deck,
                        deck.begin(), std::next(deck.begin(), position));
        } else if (position < 0) {
            deck.splice(deck.begin(), deck,
                        std::prev(deck.end(), -position), deck.end());
        }
    }

    long long simulate_fast(long long deck_size,
                            long long starting_pos) const override {
        auto result = starting_pos - position;
        if (result < 0) {
            result += deck_size;
        } else if (result >= deck_size) {
            result %= deck_size;
        }
        return result;
    }

    int position = 0;
};


using instructions_type = std::vector<std::shared_ptr<Instruction> >;


void simulate_slow(deck_type &deck, const instructions_type &instructions) {
    for (auto &instr: instructions) {
        instr->simulate_slow(deck);
    }
}


long long simulate_fast(long long deck_size, long long starting_pos,
                        const instructions_type &instructions) {
    for (auto &instr: instructions) {
        starting_pos = instr->simulate_fast(deck_size, starting_pos);
    }
    return starting_pos;
}


void print_deck(const deck_type &deck) {
    for (auto &item: deck) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);

    instructions_type instructions;
    std::string line;
    while (std::getline(input_stream, line)) {
        std::smatch match;
        if (std::regex_match(line, match, DEAL_STACK_REGEX)) {
            instructions.push_back(std::make_shared<DealIntoNewStack>());
        } else if (std::regex_match(line, match, DEAL_INCREMENT_REGEX)) {
            instructions.push_back(std::make_shared<DealWithIncrement>(
                std::stoi(match.str(1))));
        } else if (std::regex_match(line, match, CUT_REGEX)) {
            instructions.push_back(std::make_shared<Cut>(
                std::stoi(match.str(1))));
        } else {
            std::cerr << "Invalid line: " << line << std::endl;
            exit(3);
        }
    }

    // deck_type deck;
    // for (long long i = 0; i < NUM_CARDS; ++i) {
    //     deck.push_back(i);
    // }
    // simulate_slow(deck, instructions);
    // long long part1_result = -1;
    // for (auto iter = deck.begin(); iter != deck.end(); ++iter) {
    //     if (*iter == DESIRED_CARD) {
    //         part1_result = std::distance(deck.begin(), iter);
    //         break;
    //     }
    // }

    auto part1_result = simulate_fast(PART1_DECK_SIZE, PART1_DESIRED_CARD,
                                      instructions);

    long long part2_result = PART2_DESIRED_CARD;
    for (long long i = 0; i < PART2_REPEATS; ++i) {
        std::cout << i << '\r';
        part2_result = simulate_fast(PART2_DECK_SIZE, part2_result,
                                     instructions);
    }
    std::cout << std::endl;

    std::cout << "PART 1" << std::endl;
    std::cout << "Position of card " << PART1_DESIRED_CARD << ": ";
    std::cout << part1_result << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Position of card " << PART2_DESIRED_CARD << ": ";
    std::cout << part2_result << std::endl;
    return 0;
}
