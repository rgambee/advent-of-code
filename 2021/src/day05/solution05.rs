use crate::util;
use regex::Regex;
use std::collections::HashMap;
use std::fs;
use std::path;

fn sign(num: i64) -> i64 {
    match num {
        n if n > 0 => 1,
        n if n == 0 => 0,
        n if n < 0 => -1,
        _ => panic!("Unable to determine sign of {}", num),
    }
}

fn insert_point(grid: &mut HashMap<(i64, i64), i64>, point: (i64, i64)) -> i64 {
    let intersections = grid.entry(point).or_insert(0);
    *intersections += 1;
    match *intersections {
        2 => 1,
        _ => 0,
    }
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    // Example:
    // 0,9 -> 5,9
    let line_regex = Regex::new(r"^(?P<x1>\d+),(?P<y1>\d+) -> (?P<x2>\d+),(?P<y2>\d+)$").unwrap();

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let line_iter = contents.lines();

    let mut aa_intersections = HashMap::new();
    let mut all_intersections = HashMap::new();
    let mut aa_overlaps = 0;
    let mut all_overlaps = 0;
    for line in line_iter {
        let captures = line_regex.captures(line);
        if captures.is_none() {
            println!("Unable to parse line: {}", line);
            continue;
        }
        let captures = captures.unwrap();
        let x1: i64 = captures.name("x1").unwrap().as_str().parse().unwrap();
        let y1: i64 = captures.name("y1").unwrap().as_str().parse().unwrap();
        let x2: i64 = captures.name("x2").unwrap().as_str().parse().unwrap();
        let y2: i64 = captures.name("y2").unwrap().as_str().parse().unwrap();
        let axis_aligned = x1 == x2 || y1 == y2;
        let x_inc = sign(x2 - x1);
        let y_inc = sign(y2 - y1);
        let mut xi = x1;
        let mut yi = y1;
        while xi != x2 || yi != y2 {
            all_overlaps += insert_point(&mut all_intersections, (xi, yi));
            if axis_aligned {
                aa_overlaps += insert_point(&mut aa_intersections, (xi, yi));
            }
            xi += x_inc;
            yi += y_inc;
        }
        all_overlaps += insert_point(&mut all_intersections, (x2, y2));
        if axis_aligned {
            aa_overlaps += insert_point(&mut aa_intersections, (x2, y2));
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number of points overlapped by axis-aligned lines"),
            answer: aa_overlaps,
        }),
        Some(util::PartialSolution {
            message: String::from("Number of points overlapped by all lines"),
            answer: all_overlaps,
        }),
    )
}
