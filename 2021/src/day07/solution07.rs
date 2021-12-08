use crate::util;
use std::cmp;
use std::fs;
use std::path;

fn calc_fuel_part2(position: i64, mean: i64) -> i64 {
    let distance = (position - mean).abs();
    (distance * distance + distance) / 2
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut positions: Vec<i64> = contents
        .split(',')
        .map(|s| s.trim_end_matches('\n').parse().unwrap())
        .collect();
    let mean = (positions.iter().sum::<i64>() as f64) / (positions.len() as f64);
    println!("Mean position: {}", mean);

    positions.sort_unstable();
    let half_length = positions.len() / 2;
    let mut median = positions[half_length];
    if positions.len() % 2 == 0 {
        median = (positions[half_length - 1] + positions[half_length]) / 2;
    }
    println!("Median position: {}", median);

    let fuel_cost_part1 = positions.iter().fold(0, |acc, p| acc + (p - median).abs());
    // If the mean isn't an integer, try rounding up and down
    // to see which results in the smaller cost.
    let fuel_cost_part2_floor = positions
        .iter()
        .fold(0, |acc, p| acc + calc_fuel_part2(*p, mean.floor() as i64));
    let fuel_cost_part2_ceil = positions
        .iter()
        .fold(0, |acc, p| acc + calc_fuel_part2(*p, mean.ceil() as i64));
    let fuel_cost_part2 = cmp::min(fuel_cost_part2_floor, fuel_cost_part2_ceil);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Fuel required to align positions"),
            answer: fuel_cost_part1,
        }),
        Some(util::PartialSolution {
            message: String::from("Fuel required to align positions"),
            answer: fuel_cost_part2,
        }),
    )
}
