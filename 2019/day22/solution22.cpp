#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
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
    virtual void simulate_inverse_slow(deck_type &deck) const = 0;
    virtual long long simulate_fast(long long deck_size,
                                    long long starting_pos) const = 0;
    virtual long long simulate_inverse_fast(long long deck_size,
                                            long long ending_pos) const = 0;
};


class DealIntoNewStack: public Instruction {
public:
    void simulate_slow(deck_type &deck) const override {
        deck.reverse();
    }

    void simulate_inverse_slow(deck_type &deck) const override {
        simulate_slow(deck);
    }

    long long simulate_fast(long long deck_size,
                            long long starting_pos) const override {
        return deck_size - starting_pos - 1;
    }

    long long simulate_inverse_fast(long long deck_size,
                                    long long ending_pos) const override {
        return simulate_fast(deck_size, ending_pos);
    }
};


class DealWithIncrement: public Instruction {
public:
    DealWithIncrement(long long inc): increment(inc) {}

    void simulate_slow(deck_type &deck) const override {
        auto copy = deck;
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

    void simulate_inverse_slow(deck_type &deck) const override {
        auto copy = deck;
        auto source = copy.begin();
        for (auto card = deck.begin(); card != deck.end(); ++card) {
            *card = *source;
            for (long long i = 0; i < increment; ++i) {
                std::advance(source, 1);
                if (source == copy.end()) {
                    source = copy.begin();
                }
            }
        }
    }

    long long simulate_fast(long long deck_size,
                            long long starting_pos) const override {
        return (starting_pos * increment) % deck_size;
    }

    long long simulate_inverse_fast(long long deck_size,
                                    long long ending_pos) const override {
        while (ending_pos % increment) {
            ending_pos += deck_size;
        }
        return ending_pos / increment;
    }

    long long increment = 1;
};


class Cut: public Instruction {
public:
    Cut(long long pos): position(pos) {}

    void simulate_slow(deck_type &deck, long long cut_position) const {
        if (cut_position > 0) {
            deck.splice(deck.end(), deck,
                        deck.begin(), std::next(deck.begin(), cut_position));
        } else if (cut_position < 0) {
            deck.splice(deck.begin(), deck,
                        std::prev(deck.end(), -cut_position), deck.end());
        }
    }

    void simulate_slow(deck_type &deck) const override {
        simulate_slow(deck, position);
    }

    void simulate_inverse_slow(deck_type &deck) const override {
        simulate_slow(deck, -position);
    }

    long long simulate_fast(long long deck_size,
                            long long starting_pos,
                            long long cut_position) const {
        auto result = starting_pos - cut_position;
        if (result < 0) {
            result += deck_size;
        } else if (result >= deck_size) {
            result %= deck_size;
        }
        return result;
    }

    long long simulate_fast(long long deck_size,
                            long long starting_pos) const override {
        return simulate_fast(deck_size, starting_pos, position);
    }

    long long simulate_inverse_fast(long long deck_size,
                                    long long ending_pos) const override {
        return simulate_fast(deck_size, ending_pos, -position);
    }

    long long position = 0;
};


using instructions_type = std::vector<std::shared_ptr<Instruction> >;


void simulate_slow(deck_type &deck, const instructions_type &instructions) {
    for (auto &instr: instructions) {
        instr->simulate_slow(deck);
    }
}


void simulate_inverse_slow(deck_type &deck,
                           const instructions_type &instructions) {
    for (auto iter = instructions.rbegin(); iter != instructions.rend(); ++iter) {
        (*iter)->simulate_inverse_slow(deck);
    }
}


long long simulate_fast(long long deck_size, long long starting_pos,
                        const instructions_type &instructions) {
    for (auto &instr: instructions) {
        starting_pos = instr->simulate_fast(deck_size, starting_pos);
    }
    return starting_pos;
}


long long simulate_inverse_fast(long long deck_size, long long ending_pos,
                                const instructions_type &instructions) {
    for (auto iter = instructions.rbegin(); iter != instructions.rend(); ++iter) {
        ending_pos = (*iter)->simulate_inverse_fast(deck_size, ending_pos);
    }
    return ending_pos;
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
            std::stringstream error_message;
            error_message << "Invalid line: " << line;
            throw std::runtime_error(error_message.str());
        }
    }

    // auto part1_result = simulate_fast(PART1_DECK_SIZE, PART1_DESIRED_CARD,
    //                                   instructions);

    long long end_pos = PART2_DESIRED_CARD;
    for (long long i = 0; i < PART2_REPEATS; ++i) {
        std::cout << end_pos << std::endl;
        end_pos = simulate_inverse_fast(PART2_DECK_SIZE, end_pos, instructions);
    }
    std::cout << end_pos << std::endl;
    return 0;


    long long part2_position = PART2_DESIRED_CARD;
    auto num_iterations = PART2_REPEATS;
    long long period = -1;
    std::unordered_map<long long, long long> past_positions;
    for (long long i = 0; i < num_iterations; ++i) {
        if (period < 0
            && past_positions.find(part2_position) != past_positions.end()) {
            auto period = i - past_positions.at(part2_position);
            std::cout << std::endl << "Cycle found, period: " << period << std::endl;
            num_iterations = num_iterations % period + i;
        } else {
            past_positions[part2_position] = i;
            part2_position = simulate_fast(PART2_DECK_SIZE, part2_position,
                                           instructions);
        }
    }
    std::cout << part2_position << std::endl;
    std::cout << std::endl;

    std::cout << "PART 1" << std::endl;
    std::cout << "Position of card " << PART1_DESIRED_CARD << ": ";
    std::cout << part1_result << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Position of card " << PART2_DESIRED_CARD << ": ";
    std::cout << part2_position << std::endl;
    return 0;
}
