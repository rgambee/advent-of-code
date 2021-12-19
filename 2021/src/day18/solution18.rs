use crate::util;
use std::boxed::Box;
use std::cmp;
use std::collections::VecDeque;
use std::fmt;
use std::fs;
use std::path;

#[derive(Clone)]
enum Number {
    Leaf(i64),
    Node {
        left: Box<Number>,
        right: Box<Number>,
    },
}

#[derive(Clone, Debug, PartialEq)]
enum Direction {
    Left,
    Right,
}

impl Number {
    fn resolve_path(&mut self, path: &[Direction]) -> Result<&mut Number, &str> {
        if path.is_empty() {
            return Ok(self);
        }
        match self {
            Number::Leaf(_) => Err("Invalid path"),
            Number::Node { left, right } => match path[0] {
                Direction::Left => left.resolve_path(&path[1..]),
                Direction::Right => right.resolve_path(&path[1..]),
            },
        }
    }

    fn paths_to_leaves(&self) -> Vec<VecDeque<Direction>> {
        let mut paths: Vec<VecDeque<Direction>> = Vec::new();
        match self {
            Number::Leaf(_) => {
                paths.push(VecDeque::new());
            }
            Number::Node { left, right } => {
                let mut left_paths = left.paths_to_leaves();
                let mut right_paths = right.paths_to_leaves();
                for p in left_paths.iter_mut() {
                    p.push_front(Direction::Left);
                }
                for p in right_paths.iter_mut() {
                    p.push_front(Direction::Right);
                }
                paths.append(&mut left_paths);
                paths.append(&mut right_paths);
            }
        };
        paths
    }

    fn explode(&mut self) -> bool {
        let paths = self.paths_to_leaves();
        for (i, path) in paths.iter().enumerate() {
            if path.len() <= 4 {
                // This leaf isn't deep enough. We check whether it's <= 4
                // since the path includes the leaf itself.
                continue;
            }
            // We're looking for a pair of adjacent regular numbers, which
            // means a pair of paths that differ only in their final direction.
            if i == paths.len() - 1 {
                // This is the last path, so this leaf can't be the left of a pair.
                continue;
            }
            let next_path = &paths[i + 1];

            // Convert these paths to Vecs so we can slice them. It's a shame
            // VecDeques don't support simple slicing. Below, I use
            // .clone().make_contiguous() to create slices, which seems nonideal.
            // The path points to the leaf itself, but we want its parent.
            // Therefore we strip off the final element when converting it.
            let path = &Vec::from(path.clone())[..path.len() - 1];
            let next_path = &Vec::from(next_path.clone())[..next_path.len() - 1];

            if path != next_path {
                // These paths don't point to adjacent leaves
                continue;
            }

            let left_leaf;
            let right_leaf;
            {
                let exploding_pair = self
                    .resolve_path(path)
                    .expect("Failed to resolve exploding pair");
                if let Number::Node { left, right } = exploding_pair {
                    if let Number::Leaf(l) = **left {
                        left_leaf = l;
                    } else {
                        panic!("Expected leaf but found node");
                    }
                    if let Number::Leaf(r) = **right {
                        right_leaf = r;
                    } else {
                        panic!("Expected leaf but found node")
                    }
                } else {
                    panic!("Expected node but found leaf");
                }
                *exploding_pair = Number::Leaf(0);
            }
            if i > 0 {
                let left_path = &paths[i - 1];
                if let Ok(Number::Leaf(neigh_left)) =
                    self.resolve_path(left_path.clone().make_contiguous())
                {
                    *neigh_left += left_leaf;
                } else {
                    panic!("Failed to find left leaf");
                }
            }
            // Here we look two paths ahead since we want to skip
            // the right leaf of the exploding pair.
            if let Some(right_path) = paths.get(i + 2) {
                if let Ok(Number::Leaf(neigh_right)) =
                    self.resolve_path(right_path.clone().make_contiguous())
                {
                    *neigh_right += right_leaf;
                } else {
                    panic!(
                        "Failed to find right leaf from path {:?}\nSelf is {}",
                        right_path, self
                    );
                }
            }
            return true;
        }
        false
    }

    fn split(&mut self) -> bool {
        for path in self.paths_to_leaves().iter() {
            let node = self.resolve_path(path.clone().make_contiguous()).unwrap();
            if let Number::Leaf(value) = node {
                if *value >= 10 {
                    *node = Number::Node {
                        left: Box::new(Number::Leaf((*value) / 2)),
                        right: Box::new(Number::Leaf((*value + 1) / 2)),
                    };
                    return true;
                }
            }
        }
        false
    }

    fn reduce(&mut self) {
        loop {
            if self.explode() {
                continue;
            }
            if self.split() {
                continue;
            }
            break;
        }
    }

    fn parse_from_iter(iter: &mut impl Iterator<Item = char>) -> Number {
        let mut next_char = iter.next().unwrap();
        if let Some(num) = next_char.to_digit(10) {
            return Number::Leaf(num as i64);
        }
        if next_char == '[' {
            let left_num = Number::parse_from_iter(iter);
            next_char = iter.next().unwrap();
            if next_char != ',' {
                panic!("Expected ',' but found '{}'", next_char);
            }
            let right_num = Number::parse_from_iter(iter);
            next_char = iter.next().unwrap();
            if next_char != ']' {
                panic!("Expected ']' but found '{}'", next_char);
            }
            return Number::Node {
                left: Box::new(left_num),
                right: Box::new(right_num),
            };
        }
        panic!("Unexpected character '{}'", next_char);
    }

    fn calculate_magnitude(&self) -> i64 {
        match self {
            Number::Leaf(n) => *n,
            Number::Node { left, right } => {
                3 * left.calculate_magnitude() + 2 * right.calculate_magnitude()
            }
        }
    }
}

impl fmt::Display for Number {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Number::Leaf(n) => write!(f, "{}", n),
            Number::Node { left, right } => {
                write!(f, "[{},{}]", left, right)
            }
        }
    }
}

fn add(left: Number, right: Number) -> Number {
    let mut sum = Number::Node {
        left: Box::new(left),
        right: Box::new(right),
    };
    sum.reduce();
    sum
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));

    let mut numbers: Vec<Number> = Vec::new();
    for line in contents.lines() {
        numbers.push(Number::parse_from_iter(&mut line.chars()));
    }

    let mut running_sum = numbers.get(0).expect("No numbers parsed").clone();
    let mut max_magnitude = 0;
    for (i, numi) in numbers.iter().enumerate() {
        if i > 0 {
            running_sum = add(running_sum, numi.clone());
        }
        for (j, numj) in numbers.iter().enumerate() {
            if i == j {
                continue;
            }
            max_magnitude = cmp::max(
                add(numi.clone(), numj.clone()).calculate_magnitude(),
                max_magnitude,
            );
        }
    }

    let grand_total_magnitude = running_sum.calculate_magnitude();

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Magnitude of sum"),
            answer: grand_total_magnitude,
        }),
        Some(util::PartialSolution {
            message: String::from("Maximum magnitude for any pairwise sum"),
            answer: max_magnitude,
        }),
    )
}
