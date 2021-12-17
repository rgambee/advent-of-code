use crate::util::{self, Grid2D, GridSlice2D, Point2D};
use std::cmp;
use std::collections::{BinaryHeap, HashMap};
use std::fs;
use std::path;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
struct Node {
    location: Point2D,
    previous: Option<Point2D>,
    cumulative_risk: i64,
}

impl Ord for Node {
    fn cmp(&self, other: &Self) -> cmp::Ordering {
        other.cumulative_risk.cmp(&self.cumulative_risk)
    }
}

impl PartialOrd for Node {
    fn partial_cmp(&self, other: &Self) -> Option<cmp::Ordering> {
        Some(self.cmp(other))
    }
}

fn find_best_route(grid: &GridSlice2D) -> i64 {
    // This uses Dijkstra's algorithm
    // https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
    let start = (0, 0);
    let exit = (grid.len() - 1, grid[0].len() - 1);
    let mut node_map: HashMap<Point2D, Node> = HashMap::new();
    for (r, row) in grid.iter().enumerate() {
        for c in 0..row.len() {
            node_map.insert(
                (r, c),
                Node {
                    location: (r, c),
                    previous: None,
                    cumulative_risk: i64::MAX,
                },
            );
        }
    }
    node_map.get_mut(&start).unwrap().cumulative_risk = 0;
    let mut node_heap: BinaryHeap<Node> = BinaryHeap::from([*node_map.get(&start).unwrap()]);
    while !node_heap.is_empty() {
        let curr_node = node_heap.pop().unwrap();
        for neigh_pos in util::get_cardinal_neighbors(grid, curr_node.location) {
            let mut neigh_node = node_map.get_mut(&neigh_pos).unwrap();
            let risk_from_current = curr_node.cumulative_risk + util::at(grid, neigh_pos).unwrap();
            if risk_from_current < neigh_node.cumulative_risk {
                neigh_node.cumulative_risk = risk_from_current;
                node_heap.push(*neigh_node);
            }
        }
    }
    node_map.get(&exit).unwrap().cumulative_risk
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut grid_part1: Grid2D = Vec::new();
    for line in contents.lines() {
        let row = line
            .chars()
            .map(|c| c.to_digit(10).unwrap() as i64)
            .collect();
        grid_part1.push(row);
    }
    let mut grid_part2: Grid2D = Vec::new();
    for i in 0..5 {
        for row1 in grid_part1.iter() {
            let mut row2: Vec<i64> = Vec::new();
            for j in 0..5 {
                for cell in row1.iter() {
                    row2.push((cell + i + j - 1) % 9 + 1);
                }
            }
            grid_part2.push(row2);
        }
    }
    assert!(grid_part2.len() == grid_part1.len() * 5);
    assert!(grid_part2[0].len() == grid_part1[0].len() * 5);

    let lowest_risk_part1 = find_best_route(&grid_part1);
    let lowest_risk_part2 = find_best_route(&grid_part2);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Lowest total risk for partial cave"),
            answer: lowest_risk_part1,
        }),
        Some(util::PartialSolution {
            message: String::from("Lowest total risk for full cave"),
            answer: lowest_risk_part2,
        }),
    )
}
