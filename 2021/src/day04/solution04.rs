use crate::util;
use std::fmt;
use std::fs;
use std::path;

const BOARD_SIZE: usize = 5;

#[derive(Clone, Copy, Debug)]
struct BingoCell {
    value: i64,
    marked: bool,
}

impl fmt::Display for BingoCell {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.marked {
            write!(f, "|{}|", self.value)
        } else {
            write!(f, " {} ", self.value)
        }
    }
}

#[derive(Debug)]
struct BingoBoard {
    grid: [[BingoCell; BOARD_SIZE]; BOARD_SIZE],
    won: bool,
}

impl BingoBoard {
    fn new(chunk: &str) -> BingoBoard {
        let mut bb = BingoBoard {
            grid: [[BingoCell {
                value: 0,
                marked: false,
            }; BOARD_SIZE]; BOARD_SIZE],
            won: false,
        };
        for (i, line) in chunk.lines().enumerate() {
            for (j, num) in line.split_whitespace().enumerate() {
                bb.grid[i][j].value = num.parse().expect("Unable to parse bingo board");
            }
        }
        bb
    }

    fn mark(&mut self, value: i64) -> bool {
        let mut winner = false;
        let mut columns_complete = [true; BOARD_SIZE];
        for row in self.grid.iter_mut() {
            let mut row_complete = true;
            for (i, cell) in row.iter_mut().enumerate() {
                if cell.value == value {
                    // println!("Match found for {} in col {}", value, i);
                    cell.marked = true;
                }
                row_complete &= cell.marked;
                columns_complete[i] &= cell.marked;
            }
            winner |= row_complete;
        }
        self.won = columns_complete
            .iter()
            .fold(winner, |acc, col_winner| acc || *col_winner);
        self.won
    }

    fn add_unmarked_cells(&self) -> i64 {
        let mut sum = 0;
        for row in &self.grid {
            for cell in row {
                if !cell.marked {
                    sum += cell.value;
                }
            }
        }
        sum
    }
}

impl fmt::Display for BingoBoard {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        for row in &self.grid {
            for cell in row {
                write!(f, "{}", cell)?;
            }
            writeln!(f)?;
        }
        Ok(())
    }
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut chunk_iter = contents.split("\n\n");
    let drawn_numbers: Vec<i64> = chunk_iter
        .next()
        .expect("Unable to read drawn numbers")
        .split(',')
        .map(|s| s.parse().expect("Unable to parse drawn number"))
        .collect();

    let mut boards: Vec<BingoBoard> = Vec::new();
    for chunk in chunk_iter {
        boards.push(BingoBoard::new(chunk));
    }

    let mut first_score: Option<i64> = None;
    let mut last_score: Option<i64> = None;
    let mut win_count = 0;
    let board_count = boards.len();
    for value in drawn_numbers {
        for board in boards.iter_mut() {
            if board.won {
                continue;
            }
            if board.mark(value) {
                win_count += 1;
                if first_score.is_none() {
                    first_score = Some(board.add_unmarked_cells() * value);
                }
                if win_count == board_count {
                    last_score = Some(board.add_unmarked_cells() * value);
                }
            }
        }
    }
    let first_score = first_score.expect("No winning boards after all numbers drawn");
    let last_score = last_score.expect("Not all boards won after all numbers drawn");

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Score of first winning board"),
            answer: first_score,
        }),
        Some(util::PartialSolution {
            message: String::from("Score of last winning board"),
            answer: last_score,
        }),
    )
}
