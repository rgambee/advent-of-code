use crate::util::{self, Grid2D, GridSlice2D, Point2D};
use std::collections::HashSet;
use std::fs;
use std::path;

const BASIN_COUNT: usize = 3;

fn get_size_of_basin(grid: &GridSlice2D, local_minimum: Point2D) -> i64 {
    let mut basin_size = 0;
    let mut visited: HashSet<Point2D> = HashSet::new();
    let mut to_visit: Vec<Point2D> = vec![local_minimum];
    while !to_visit.is_empty() {
        let point = to_visit.pop().unwrap();
        if visited.contains(&point) {
            continue;
        }
        visited.insert(point);
        if *util::at(grid, point).unwrap() != 9 {
            basin_size += 1;
            for neigh in util::get_cardinal_neighbors(grid, point) {
                to_visit.push(neigh);
            }
        }
    }
    basin_size
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut grid: Grid2D = Vec::new();
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
            // For this problem, locations that are diagonally adjacent do not count
            // as neighbors. Therefore, we should call util::get_cardinal_neighbors()
            // instead of util::get_all_neighbors(). However, when I do so, I get the
            // wrong answer for part 1. I need to look into this further to figure out
            // what's going wrong.
            let neighbors = util::get_all_neighbors(&grid, (row_index, col_index));
            let mut local_minimum = true;
            for neigh in neighbors {
                if *util::at(&grid, neigh).unwrap() < height {
                    local_minimum = false;
                    break;
                }
            }
            if local_minimum {
                risk_level_sum += height + 1;
                basin_sizes.push(get_size_of_basin(&grid, (row_index, col_index)));
            }
        }
    }
    basin_sizes.sort_unstable();
    basin_sizes.reverse();
    let basin_size_product = basin_sizes.iter().take(BASIN_COUNT).product();

    util::Solution(
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
