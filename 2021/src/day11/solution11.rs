use crate::util::{self, GridSlice2D, Point2D};
use std::collections::HashSet;
use std::fs;
use std::path;

const NUM_STEPS_PART1: i64 = 100;
const FLASH_THRESHOLD: i64 = 10;

fn increment_neighbors(grid: &mut GridSlice2D, center: Point2D, flashed: &mut HashSet<Point2D>) {
    for neigh in util::get_all_neighbors(grid, center) {
        if flashed.contains(&neigh) {
            continue;
        }
        if let Some(oct) = util::at_mut(grid, neigh) {
            *oct += 1;
            if *oct >= FLASH_THRESHOLD {
                flashed.insert(neigh);
                *oct = 0;
                increment_neighbors(grid, neigh, flashed);
            }
        }
    }
}

fn simulate_step(grid: &mut GridSlice2D) -> usize {
    let mut flashed: HashSet<Point2D> = HashSet::new();
    // First increment all energy levels
    for r in 0..grid.len() {
        for c in 0..grid[r].len() {
            let oct = util::at_mut(grid, (r, c)).unwrap();
            *oct += 1;
            if *oct >= FLASH_THRESHOLD {
                flashed.insert((r, c));
                *oct = 0;
            }
        }
    }
    // Now let any members over the engery threshold spill over
    // into their neighbors
    let flashed_copy = flashed.clone();
    for center in flashed_copy.into_iter() {
        increment_neighbors(grid, center, &mut flashed);
    }
    flashed.len()
}

fn print_grid(grid: &[Vec<i64>]) {
    for row in grid.iter() {
        for oct in row.iter() {
            print!("{:X}", oct);
        }
        println!();
    }
    println!();
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut grid: Vec<Vec<i64>> = Vec::new();
    for line in contents.lines() {
        let row = line
            .chars()
            .map(|c| c.to_digit(10).unwrap() as i64)
            .collect();
        grid.push(row);
    }
    print_grid(&grid);

    let mut flash_sum = 0;
    let synchronized_step;
    let mut step_count = 0;
    loop {
        let flashes = simulate_step(&mut grid);
        step_count += 1;
        if step_count <= NUM_STEPS_PART1 {
            flash_sum += flashes;
        }
        if flashes == grid.len() * grid[0].len() {
            synchronized_step = step_count;
            break;
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: format!("Number of flashes after {} steps", NUM_STEPS_PART1),
            answer: flash_sum as i64,
        }),
        Some(util::PartialSolution {
            message: String::from("Number of steps needed to synchronize"),
            answer: synchronized_step,
        }),
    )
}
