use crate::util;
use std::fs;
use std::path;

fn move_cucumbers(grid: &mut [Vec<char>], facing: char, row_inc: usize, col_inc: usize) -> bool {
    let mut moved = false;
    let grid_len = grid.len();
    let mut to_move: Vec<util::Point2D> = Vec::new();
    for r in 0..grid_len {
        let row_len = grid[r].len();
        for c in 0..row_len {
            let this = grid[r][c];
            let next = grid[(r + row_inc) % grid_len][(c + col_inc) % row_len];
            if this == facing && next == '.' {
                to_move.push((r, c));
                // println!(
                //     "Moving {} from {:?} to {:?}",
                //     facing,
                //     (r, c),
                //     ((r + row_inc) % grid_len, (c + col_inc) % row_len),
                // );
                moved = true;
            } // } else if this == facing {
              //     println!(
              //         "Not moving {} from {:?} because next is {}",
              //         facing,
              //         (r, c),
              //         next,
              //     );
              // }
        }
    }
    for (r, c) in to_move.into_iter() {
        let row_len = grid[r].len();
        grid[r][c] = '.';
        grid[(r + row_inc) % grid_len][(c + col_inc) % row_len] = facing;
    }
    moved
}

fn move_east(grid: &mut [Vec<char>]) -> bool {
    move_cucumbers(grid, '>', 0, 1)
}

fn move_south(grid: &mut [Vec<char>]) -> bool {
    move_cucumbers(grid, 'v', 1, 0)
}

fn move_until_done(grid: &mut [Vec<char>]) -> i64 {
    let mut steps = 0;
    for i in 1.. {
        let mut moved = false;
        moved |= move_east(grid);
        moved |= move_south(grid);
        // print_grid(grid);
        // println!("{}", i);
        if !moved {
            steps = i;
            break;
        }
    }
    steps
}

#[allow(dead_code)]
fn print_grid(grid: &[Vec<char>]) {
    for row in grid.iter() {
        println!("{}", row.iter().copied().collect::<String>());
    }
    println!();
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut grid: Vec<Vec<char>> = Vec::new();
    for line in contents.lines() {
        let row: Vec<char> = line.chars().collect();
        grid.push(row);
    }
    let steps_until_stopped = move_until_done(&mut grid);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Time until sea cucumbers stop"),
            answer: steps_until_stopped,
        }),
        None,
    )
}
