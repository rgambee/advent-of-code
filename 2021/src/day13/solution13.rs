use crate::util::{self, Point2D};
use regex::Regex;
use std::cmp;
use std::collections::HashSet;
use std::fs;
use std::path;

enum Axis {
    X,
    Y,
}

struct Fold {
    axis: Axis,
    coord: usize,
}

fn fold_paper(dots: &HashSet<Point2D>, fold: &Fold) -> HashSet<Point2D> {
    let mut new_dots: HashSet<Point2D> = HashSet::new();
    for point in dots.iter() {
        let new_point = match fold.axis {
            Axis::X => (cmp::min(2 * fold.coord - point.0, point.0), point.1),
            Axis::Y => (point.0, cmp::min(2 * fold.coord - point.1, point.1)),
        };
        new_dots.insert(new_point);
    }
    new_dots
}

fn print_dots(dots: &HashSet<Point2D>) {
    let mut max_row = 0;
    let mut max_col = 0;
    for point in dots.iter() {
        max_row = cmp::max(max_row, point.1);
        max_col = cmp::max(max_col, point.0);
    }
    for r in 0..max_row + 1 {
        for c in 0..max_col + 1 {
            if dots.contains(&(c, r)) {
                print!("#");
            } else {
                print!(" ");
            }
        }
        println!();
    }
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    // Example:
    // 72,877
    let dot_regex = Regex::new(r"^(?P<x>\d+),(?P<y>\d+)$").unwrap();
    // Example:
    // fold along y=111
    let fold_regex = Regex::new(r"^fold along (?P<axis>x|y)=(?P<coord>\d+)$").unwrap();

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut dots: HashSet<Point2D> = HashSet::new();
    let mut folds: Vec<Fold> = Vec::new();
    for line in contents.lines() {
        if let Some(captures) = dot_regex.captures(line) {
            let point = (
                captures.name("x").unwrap().as_str().parse().unwrap(),
                captures.name("y").unwrap().as_str().parse().unwrap(),
            );
            dots.insert(point);
            continue;
        }
        if let Some(captures) = fold_regex.captures(line) {
            let axis = match captures.name("axis").unwrap().as_str() {
                "x" => Axis::X,
                "y" => Axis::Y,
                _ => panic!("Invalid axis"),
            };
            folds.push(Fold {
                axis,
                coord: captures.name("coord").unwrap().as_str().parse().unwrap(),
            });
            continue;
        }
        if !line.is_empty() {
            println!("Unrecognized line: '{}'", line);
        }
    }

    let mut num_dots_after_first_fold = 0;
    for (i, fold) in folds.iter().enumerate() {
        dots = fold_paper(&dots, fold);
        if i == 0 {
            num_dots_after_first_fold = dots.len() as i64;
        }
    }
    print_dots(&dots);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number of visible dots after first fold"),
            answer: num_dots_after_first_fold,
        }),
        Some(util::PartialSolution {
            message: String::from("Activation code shown above"),
            answer: -1,
        }),
    )
}
