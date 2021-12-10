use crate::util;
use std::collections::HashSet;
use std::fs;
use std::path;

type Point = (i64, i64);

const BASIN_COUNT: usize = 3;

fn at(grid: &[Vec<i64>], location: Point) -> Option<&i64> {
    if location.0 < 0 || location.1 < 0 {
        return None;
    }
    if let Some(row) = grid.get(location.0 as usize) {
        return row.get(location.1 as usize);
    }
    None
}

fn get_neighbors(grid: &[Vec<i64>], location: Point) -> Vec<Point> {
    let mut neighbors: Vec<Point> = Vec::new();
    for r in location.0 - 1..location.0 + 2 {
        for c in location.1 - 1..location.1 + 2 {
            // This conditional is checking two things (I guess technically 3 or 4,
            // depending how you count the tuple comparison):
            //      1. We're at the original location itself
            //      2. We're in one of the corners of the 3x3 grid
            //         (i.e. we're diagonally adjacent, not orthogonally)
            // With the code as it is now, I get the wrong answer for part 1
            // but the right answer for part 2. If I remove the second criterion,
            // I get the right answer for part 1 but the wrong answer for part 2.
            // I'm not sure why this is, so I'm leaving it for now with the hope
            // that I'll return to this (assuming I don't forget).
            if (r, c) == location || (r != location.0 && c != location.1) {
                continue;
            }
            if at(grid, (r, c)).is_some() {
                neighbors.push((r, c));
            }
        }
    }
    neighbors
}

fn get_size_of_basin(grid: &[Vec<i64>], local_minimum: Point) -> i64 {
    let mut basin_size = 0;
    let mut visited: HashSet<Point> = HashSet::new();
    let mut to_visit: Vec<Point> = vec![local_minimum];
    while !to_visit.is_empty() {
        let point = to_visit.pop().unwrap();
        if visited.contains(&point) {
            continue;
        }
        visited.insert(point);
        if *at(grid, point).unwrap() != 9 {
            basin_size += 1;
            for neigh in get_neighbors(grid, point) {
                to_visit.push(neigh);
            }
        }
    }
    basin_size
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

    let mut risk_level_sum = 0;
    let mut basin_sizes: Vec<i64> = Vec::new();
    for row_index in 0..grid.len() {
        for col_index in 0..grid[row_index].len() {
            let height = grid[row_index][col_index];
            let neighbors = get_neighbors(&grid, (row_index as i64, col_index as i64));
            let mut local_minimum = true;
            for neigh in neighbors {
                if *at(&grid, neigh).unwrap() < height {
                    local_minimum = false;
                    break;
                }
            }
            if local_minimum {
                risk_level_sum += height + 1;
                basin_sizes.push(get_size_of_basin(
                    &grid,
                    (row_index as i64, col_index as i64),
                ));
            }
        }
    }
    basin_sizes.sort_unstable();
    basin_sizes.reverse();
    let basin_size_product = basin_sizes.iter().take(BASIN_COUNT).product();

    util::Solution(
        // FIXME: part 1 solution is now wrong
        Some(util::PartialSolution {
            message: String::from("Total risk level"),
            answer: risk_level_sum,
        }),
        Some(util::PartialSolution {
            message: format!("Product of sizes of {} largest basins", BASIN_COUNT),
            answer: basin_size_product,
        }),
    )
}
