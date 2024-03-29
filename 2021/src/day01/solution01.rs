use crate::util;
use std::collections::VecDeque;
use std::fs;
use std::path;

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let line_iter = contents.lines();
    let mut recent: VecDeque<i32> = VecDeque::new();
    let mut singlet_increase_count = 0;
    let mut triplet_increase_count = 0;
    for line in line_iter {
        recent.push_back(line.parse().unwrap());
        if recent.len() > 1 && recent[recent.len() - 2] < recent[recent.len() - 1] {
            singlet_increase_count += 1;
        }
        if recent.len() > 3 {
            let oldest = recent.pop_front().unwrap();
            if oldest < *recent.back().unwrap() {
                triplet_increase_count += 1;
            }
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number of singlet increases"),
            answer: singlet_increase_count,
        }),
        Some(util::PartialSolution {
            message: String::from("Number of triplet increases"),
            answer: triplet_increase_count,
        }),
    )
}
