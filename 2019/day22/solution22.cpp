#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#include "utils.h"


// Big thanks to Spheniscine for their explanation of this problem.
// https://codeforces.com/blog/entry/72593
// I would not have solved it on my own.


using ull = unsigned long long;
constexpr ull PART1_DECK_SIZE = 10007;
constexpr ull PART1_DESIRED_CARD = 2019;
constexpr ull PART2_DECK_SIZE = 119315717514047;
constexpr ull PART2_REPEATS = 101741582076661;
constexpr ull PART2_DESIRED_POSITION = 2020;
const std::regex DEAL_STACK_REGEX("deal into new stack");
const std::regex DEAL_INCREMENT_REGEX("deal with increment ([[:digit:]]+)");
const std::regex CUT_REGEX("cut (-?[[:digit:]]+)");
using deck_type = std::list<ull>;
using comp_type = std::pair<ull, ull>;


ull add(ull a, ull b, ull modulus) {
    return (a + b) % modulus;
}


ull add(ull a, long long b, ull modulus) {
    // b may be negative
    long long result = a + b;
    while (result < 0) {
        result += modulus;
    }
    return result % modulus;
}


ull negate(ull value, ull modulus) {
    // Modular additive inverse
    return (modulus - value) % modulus;
}


ull log_2(ull value) {
    ull result = 0;
    while (value > 1) {
        value >>= 1;
        ++result;
    }
    return result;
}


ull pow(ull base, ull exponent) {
    // May overflow. Use exponentiate if that's a concern.
    ull result = 1;
    while (exponent-- > 0) {
        result *= base;
    }
    return result;
}



ull multiply(ull a, ull b, ull modulus) {
    if (a < (1ULL << 32) && b < (1ULL << 32)) {
        // No danger of overflowing
        return (a * b) % modulus;
    }
    if (a == 0 || b == 0) {
        return 0;
    }
    // Do this bit by bit to avoid overflowing
    std::vector<ull> sums(log_2(b) + 1);
    // Set sums[i] = a * 2**i
    sums[0] = a;
    for (size_t i = 1; i < sums.size(); ++i) {
        sums[i] = add(sums[i-1], sums[i-1], modulus);
    }
    ull result = 0;
    for (size_t s = 0; b > 0; ++s, b >>= 1) {
        if (b % 2) {
            result = add(result, sums[s], modulus);
        }
    }
    return result;
}


ull exponentiate(ull base, ull exponent, ull modulus) {
    if (base == 1 || base == 0) {
        return base;
    }
    if (exponent == 0) {
        return 1;
    }
    // Do this bit by bit to avoid overflowing
    std::vector<ull> powers(log_2(exponent) + 1);
    // Set powers[i] = base**(2**i)
    powers[0] = base;
    for (size_t i = 1; i < powers.size(); ++i) {
        powers[i] = multiply(powers[i-1], powers[i-1], modulus);
    }
    ull result = 1;
    for (size_t p = 0; exponent > 0; ++p, exponent >>= 1) {
        if (exponent % 2) {
            result = multiply(result, powers[p], modulus);
        }
    }
    return result;
}


ull divide(ull numerator, ull denominator, ull modulus) {
    // Compute the modular multiplicative inverse using Euler's theorem:
    // https://en.wikipedia.org/wiki/Modular_multiplicative_inverse
    // Here, we assume modulus is prime.
    ull inverse = exponentiate(denominator, modulus - 2, modulus);
    return multiply(numerator, inverse, modulus);
}


class Instruction {
public:
    virtual ~Instruction() {};
    virtual void simulate_slow(deck_type &deck) const = 0;
    virtual ull simulate_fast(ull deck_size, ull starting_pos) const = 0;
    virtual comp_type compose(const comp_type &compose_params,
                              ull deck_type) const = 0;
};


class DealIntoNewStack: public Instruction {
public:
    void simulate_slow(deck_type &deck) const override {
        deck.reverse();
    }

    ull simulate_fast(ull deck_size, ull starting_pos) const override {
        return deck_size - starting_pos - 1;
    }

    comp_type compose(const comp_type &compose_params,
                      ull deck_size) const override {
        auto multiplier = negate(compose_params.first, deck_size);
        auto offset = add(negate(compose_params.second, deck_size),
                          -1ll, deck_size);
        return comp_type(multiplier, offset);
    }
};


class DealWithIncrement: public Instruction {
public:
    DealWithIncrement(ull inc): increment(inc) {}

    void simulate_slow(deck_type &deck) const override {
        auto copy = deck;
        auto dest = deck.begin();
        for (auto card = copy.begin(); card != copy.end(); ++card) {
            *dest = *card;
            for (ull i = 0; i < increment; ++i) {
                std::advance(dest, 1);
                if (dest == deck.end()) {
                    dest = deck.begin();
                }
            }
        }
    }

    ull simulate_fast(ull deck_size, ull starting_pos) const override {
        return multiply(starting_pos, increment, deck_size);
    }

    comp_type compose(const comp_type &compose_params,
                      ull deck_size) const override {
        auto multiplier = multiply(compose_params.first, increment, deck_size);
        auto offset = multiply(compose_params.second, increment, deck_size);
        return comp_type(multiplier, offset);
    }

    ull increment = 1;
};


class Cut: public Instruction {
public:
    Cut(long long pos): position(pos) {}

    void simulate_slow(deck_type &deck) const {
        if (position > 0) {
            deck.splice(deck.end(), deck,
                        deck.begin(), std::next(deck.begin(), position));
        } else if (position < 0) {
            deck.splice(deck.begin(), deck,
                        std::prev(deck.end(), -position), deck.end());
        }
    }

    ull simulate_fast(ull deck_size, ull starting_pos) const override {
        return add(starting_pos, -position, deck_size);
    }

    comp_type compose(const comp_type &compose_params,
                      ull deck_size) const override {
        auto multiplier = compose_params.first;
        long long offset = add(compose_params.second, -position, deck_size);
        return comp_type(multiplier, offset);
    }

    long long position = 0;
};


using instructions_type = std::vector<std::shared_ptr<Instruction> >;


void simulate_slow(const instructions_type &instructions, deck_type &deck) {
    for (auto &instr: instructions) {
        instr->simulate_slow(deck);
    }
}


ull simulate_fast(const instructions_type &instructions, ull deck_size,
                  ull starting_pos) {
    for (auto &instr: instructions) {
        starting_pos = instr->simulate_fast(deck_size, starting_pos);
    }
    return starting_pos;
}


comp_type compose_all(const instructions_type &instructions, ull deck_size,
                      ull repeats = 1) {
    comp_type compose_params(1, 0);     // (multiplier, offset)
    for (auto &inst: instructions) {
        compose_params = inst->compose(compose_params, deck_size);
    }
    if (compose_params.first == 0) {
        throw std::logic_error("Composed multiplier is 0");
    }
    // Now compose this result with itself repeatedly

    // muliplier_N = pow(multiplier_0, N)
    ull multiplier = exponentiate(compose_params.first, repeats, deck_size);

    // Based on the sum of a finite geometric series:
    // https://en.wikipedia.org/wiki/Geometric_progression#Geometric_series
    // offset_N = offset_0 * (1 - pow(multiplier_0, N)) / (1 - multiplier_0)
    ull numerator = multiply(add(negate(multiplier, deck_size), 1ULL, deck_size),
                             compose_params.second, deck_size);
    ull denominator = add(negate(compose_params.first, deck_size), 1ULL, deck_size);
    ull offset = divide(numerator, denominator, deck_size);
    return comp_type(multiplier, offset);
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

    auto part1_result = simulate_fast(instructions, PART1_DECK_SIZE,
                                      PART1_DESIRED_CARD);

    auto [multiplier, offset] = compose_all(instructions, PART2_DECK_SIZE, PART2_REPEATS);
    // This tells us where each card goes after repeated shuffling:
    // final = (multiplier * initial + offset) % deck_size
    // We want the inverse:
    // initial = ((final - offset) / multiplier) % deck_size
    auto part2_result = divide(add(PART2_DESIRED_POSITION,
                                   negate(offset, PART2_DECK_SIZE),
                                   PART2_DECK_SIZE),
                               multiplier, PART2_DECK_SIZE);

    std::cout << "PART 1" << std::endl;
    std::cout << "Position of card " << PART1_DESIRED_CARD << ": ";
    std::cout << part1_result << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Card at position " << PART2_DESIRED_POSITION << ": ";
    std::cout << part2_result << std::endl;
    return 0;
}
