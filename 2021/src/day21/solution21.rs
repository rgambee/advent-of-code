use crate::util;
use regex::Regex;
use std::cmp;
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

fn quantum_branch(mut player1: Player, mut player2: Player, die_roll: i64) -> (i64, i64) {
    let active_player: &mut Player;
    let inactive_player: &mut Player;
    if player1.remaining_rolls > 0 {
        active_player = &mut player1;
        inactive_player = &mut player2;
    } else {
        active_player = &mut player2;
        inactive_player = &mut player1;
    }
    active_player.move_pawn(die_roll);
    active_player.remaining_rolls -= 1;
    if active_player.remaining_rolls == 0 {
        if active_player.update_score() >= WINNING_SCORE_PART2 {
            // println!("Game complete");
            if player1.score >= WINNING_SCORE_PART2 {
                return (1, 0);
            } else {
                return (0, 1);
            }
        } else {
            inactive_player.remaining_rolls = NUM_ROLLS_PER_TURN;
        }
    }
    let mut result = (0, 0);
    for roll in 1..4 {
        let r = quantum_branch(player1.clone(), player2.clone(), roll);
        result.0 += r.0;
        result.1 += r.1;
    }
    result
}

fn run_quantum_game(player1_starting_position: i64, player2_starting_position: i64) -> i64 {
    let player1 = Player {
        pawn_position: player1_starting_position,
        score: 0,
        remaining_rolls: NUM_ROLLS_PER_TURN,
    };
    let player2 = Player {
        pawn_position: player2_starting_position,
        score: 0,
        remaining_rolls: 0,
    };

    let mut result = (0, 0);
    for roll in 1..DIE_NUM_FACES_PART2 {
        let r = quantum_branch(player1.clone(), player2.clone(), roll);
        result.0 += r.0;
        result.1 += r.1;
    }
    cmp::max(result.0, result.1)
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
    let answer_part2 = run_quantum_game(player1_starting_position, player2_starting_position);

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
