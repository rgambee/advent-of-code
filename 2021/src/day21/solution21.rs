use crate::util;
use regex::Regex;
use std::cmp;
use std::collections::HashMap;
use std::fs;
use std::path;

const BOARD_SIZE: i64 = 10;
const NUM_ROLLS_PER_TURN: i64 = 3;
const DIE_NUM_FACES_PART1: i64 = 1000;
const DIE_NUM_FACES_PART2: i64 = 3;
const WINNING_SCORE_PART1: i64 = 1000;
const WINNING_SCORE_PART2: i64 = 21;

#[derive(Clone)]
struct Player {
    pawn_position: i64,
    score: i64,
    remaining_rolls: i64,
}

// Order is (pawn_position, score).
// I'd like to use a struct, but in order to use a struct as a HashMap key,
// I'd have to implement Hash(), which I don't how to do intelligently.
type QuantumState = (i64, i64);

impl Player {
    fn take_deterministic_turn(&mut self, die: &mut DeterministicDie) -> i64 {
        let mut die_total = 0;
        for _ in 0..self.remaining_rolls {
            die_total += die.roll();
        }
        self.move_pawn(die_total);
        self.update_score()
    }

    fn move_pawn(&mut self, die_roll: i64) {
        self.pawn_position = (self.pawn_position + die_roll - 1) % BOARD_SIZE + 1;
    }

    fn update_score(&mut self) -> i64 {
        self.score += self.pawn_position;
        self.score
    }
}

struct DeterministicDie {
    last_roll: i64,
    roll_count: i64,
}

impl DeterministicDie {
    fn roll(&mut self) -> i64 {
        self.last_roll = (self.last_roll + 1 - 1) % DIE_NUM_FACES_PART1 + 1;
        self.roll_count += 1;
        self.last_roll
    }
}

fn run_deterministic_game(player1_starting_position: i64, player2_starting_position: i64) -> i64 {
    let mut player1 = Player {
        pawn_position: player1_starting_position,
        score: 0,
        remaining_rolls: NUM_ROLLS_PER_TURN,
    };
    let mut player2 = Player {
        pawn_position: player2_starting_position,
        score: 0,
        remaining_rolls: NUM_ROLLS_PER_TURN,
    };
    let mut die = DeterministicDie {
        last_roll: 0,
        roll_count: 0,
    };

    let losing_player_score;
    loop {
        if player1.take_deterministic_turn(&mut die) >= WINNING_SCORE_PART1 {
            losing_player_score = player2.score;
            break;
        }
        player1.remaining_rolls = NUM_ROLLS_PER_TURN;
        if player2.take_deterministic_turn(&mut die) >= WINNING_SCORE_PART1 {
            losing_player_score = player1.score;
            break;
        }
        player2.remaining_rolls = NUM_ROLLS_PER_TURN;
    }
    losing_player_score * die.roll_count
}

fn count_quantum_roll_totals(num_rolls: i64) -> HashMap<i64, i64> {
    let mut totals: HashMap<i64, i64> = HashMap::new();
    if num_rolls == 1 {
        for i in 1..DIE_NUM_FACES_PART2 + 1 {
            totals.insert(i, 1);
        }
    } else {
        let subtotals = count_quantum_roll_totals(num_rolls - 1);
        for (tot, count) in subtotals.iter() {
            for i in 1..DIE_NUM_FACES_PART2 + 1 {
                *totals.entry(tot + i).or_insert(0) += count;
            }
        }
    }
    totals
}

fn update_quantum_state_possibilities(
    possible_states: &HashMap<QuantumState, i64>,
    roll_total_counts: &HashMap<i64, i64>,
) -> HashMap<QuantumState, i64> {
    let mut new_possible_states: HashMap<QuantumState, i64> = HashMap::new();
    for (state, num_universes) in possible_states.iter() {
        for (roll_total, count) in roll_total_counts.iter() {
            let new_position = (state.0 + roll_total - 1) % BOARD_SIZE + 1;
            let new_state = (new_position, state.1 + new_position);
            *new_possible_states.entry(new_state).or_insert(0) += num_universes * count;
        }
    }
    new_possible_states
}

fn count_wins(
    states_a: &mut HashMap<QuantumState, i64>,
    states_b: &HashMap<QuantumState, i64>,
) -> i64 {
    let states_a_copy = states_a.clone();
    let mut wins_a = 0;
    for (state, count) in states_a_copy.iter() {
        if state.1 >= WINNING_SCORE_PART2 {
            wins_a += count;
            states_a.remove(state).unwrap();
        }
    }
    let mut losses_b = 0;
    for (state, count) in states_b.iter() {
        if state.1 < WINNING_SCORE_PART2 {
            losses_b += count;
        }
    }
    wins_a * losses_b
}

fn simulate_quantum_outcomes(
    player1_starting_position: i64,
    player2_starting_position: i64,
    quantum_rolls: &HashMap<i64, i64>,
) -> (i64, i64) {
    let mut player1_quantum_states = HashMap::from([((player1_starting_position, 0), 1)]);
    let mut player2_quantum_states = HashMap::from([((player2_starting_position, 0), 1)]);
    let mut player1_winning_universes = 0;
    let mut player2_winning_universes = 0;
    while !player1_quantum_states.is_empty() && !player2_quantum_states.is_empty() {
        player1_quantum_states =
            update_quantum_state_possibilities(&player1_quantum_states, quantum_rolls);
        player1_winning_universes +=
            count_wins(&mut player1_quantum_states, &player2_quantum_states);
        player2_quantum_states =
            update_quantum_state_possibilities(&player2_quantum_states, quantum_rolls);
        player2_winning_universes +=
            count_wins(&mut player2_quantum_states, &player1_quantum_states);
    }
    (player1_winning_universes, player2_winning_universes)
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    // Example:
    // Player 1 starting position: 7
    let line_regex = Regex::new(r"^Player \d+ starting position: (?P<pos>\d+)$").unwrap();

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut line_iter = contents.lines();
    let captures = line_regex
        .captures(
            line_iter
                .next()
                .expect("Failed to find player 1 starting position"),
        )
        .expect("Failed to parse player 1 starting position");
    let player1_starting_position = captures.name("pos").unwrap().as_str().parse().unwrap();
    let captures = line_regex
        .captures(
            line_iter
                .next()
                .expect("Failed to find player 1 starting position"),
        )
        .expect("Failed to parse player 1 starting position");
    let player2_starting_position = captures.name("pos").unwrap().as_str().parse().unwrap();

    let answer_part1 = run_deterministic_game(player1_starting_position, player2_starting_position);

    let quantum_rolls = count_quantum_roll_totals(NUM_ROLLS_PER_TURN);
    let result = simulate_quantum_outcomes(
        player1_starting_position,
        player2_starting_position,
        &quantum_rolls,
    );
    let answer_part2 = cmp::max(result.0, result.1);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Product of losing score and last die roll"),
            answer: answer_part1,
        }),
        Some(util::PartialSolution {
            message: String::from("Number of universes more successful player wins in"),
            answer: answer_part2,
        }),
    )
}
