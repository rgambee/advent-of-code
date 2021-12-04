use crate::util;
use std::fs;
use std::iter::FromIterator;
use std::path;

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let line_iter = contents.lines();
    let mut horizontal = 0;
    let mut depth_part1 = 0;
    let mut depth_part2 = 0;
    for line in line_iter {
        let words = Vec::from_iter(line.split_whitespace());
        if words.len() != 2 {
            println!("Failed to parse line, skipping: {}", line);
            continue;
        }
        let dir = words[0];
        let dist = words[1].parse();
        if dist.is_err() {
            println!("Failed to parse distance, skipping: {}", line);
            continue;
        }
        let dist: i64 = dist.unwrap();
        match dir {
            "forward" => {
                horizontal += dist;
                // aim is equivalent to part 1's notion of depth
                depth_part2 += depth_part1 * dist;
            }
            "up" => depth_part1 -= dist,
            "down" => depth_part1 += dist,
            _ => {
                println!("Failed to parse direction, skipping: {}", line);
                continue;
            }
        };
    }
    let part1_product = depth_part1 * horizontal;
    let part2_product = depth_part2 * horizontal;

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Product of depth and horizontal distance"),
            answer: part1_product,
        }),
        Some(util::PartialSolution {
            message: String::from("Product of depth and horizontal distance"),
            answer: part2_product,
        }),
    )
}
