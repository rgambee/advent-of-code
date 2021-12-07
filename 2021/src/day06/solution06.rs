use crate::util;
use std::cmp;
use std::collections::VecDeque;
use std::fs;
use std::path;

const REPRODUCTION_PERIOD: usize = 7;
const MATURATION_PERIOD: usize = 9;
const PART_1_DURATION: u32 = 80;
const PART_2_DURATION: u32 = 256;

fn simulate_day(fish: &mut VecDeque<i64>) {
    fish.rotate_left(1);
    *fish.get_mut(REPRODUCTION_PERIOD - 1).unwrap() +=
        *fish.get_mut(MATURATION_PERIOD - 1).unwrap();
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));

    let mut fish: VecDeque<i64> = VecDeque::from([0; MATURATION_PERIOD]);
    for days_remaining in contents.split(',') {
        let days_remaining: usize = days_remaining.trim_end_matches('\n').parse().unwrap();
        *fish.get_mut(days_remaining).unwrap() += 1;
    }

    let mut part_1_number_of_fish = 0;
    let mut part_2_number_of_fish = 0;
    for day in 0..cmp::max(PART_1_DURATION, PART_2_DURATION) {
        simulate_day(&mut fish);
        if day == PART_1_DURATION - 1 {
            part_1_number_of_fish = fish.iter().sum();
        }
        if day == PART_2_DURATION - 1 {
            part_2_number_of_fish = fish.iter().sum();
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: format!("Number of lanternfish after {} days", PART_1_DURATION),
            answer: part_1_number_of_fish,
        }),
        Some(util::PartialSolution {
            message: format!("Number of lanternfish after {} days", PART_2_DURATION),
            answer: part_2_number_of_fish,
        }),
    )
}
