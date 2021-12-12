use crate::util;
use std::collections::{HashMap, HashSet};
use std::fs;
use std::path;

const START_CAVE: &str = "start";
const END_CAVE: &str = "end";

fn count_routes(
    cave_links: &HashMap<&str, HashSet<&str>>,
    route_so_far: &[&str],
    allow_small_cave_revisit: bool,
) -> usize {
    let current_cave = route_so_far.last().expect("Route so far is empty");
    if *current_cave == END_CAVE {
        return 1;
    }
    let mut route_count_from_here = 0;
    if let Some(linked_caves) = cave_links.get(current_cave) {
        for possible_dest in linked_caves.iter() {
            let is_uppercase = possible_dest.starts_with(|c| ('A'..='Z').contains(&c));
            let mut new_route = route_so_far.to_vec();
            new_route.push(possible_dest);
            if is_uppercase || !route_so_far.contains(possible_dest) {
                route_count_from_here +=
                    count_routes(cave_links, &new_route, allow_small_cave_revisit);
            } else if allow_small_cave_revisit
                && *possible_dest != START_CAVE
                && route_so_far
                    .iter()
                    .filter(|&cave| cave == possible_dest)
                    .count()
                    == 1
            {
                route_count_from_here += count_routes(cave_links, &new_route, false);
            }
        }
    }
    route_count_from_here
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut cave_links: HashMap<&str, HashSet<&str>> = HashMap::new();
    for line in contents.lines() {
        let mut cave_names = line.split('-');
        if let Some(left) = cave_names.next() {
            if let Some(right) = cave_names.next() {
                // All links are bi-directional
                cave_links
                    .entry(left)
                    .or_insert_with(HashSet::new)
                    .insert(right);
                cave_links
                    .entry(right)
                    .or_insert_with(HashSet::new)
                    .insert(left);
            } else {
                println!("Failed to parse line '{}'", line);
            }
        } else {
            println!("Failed to parse line '{}'", line);
        }
    }

    let num_routes_part1 = count_routes(&cave_links, &[START_CAVE], false);
    let num_routes_part2 = count_routes(&cave_links, &[START_CAVE], true);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number paths without revisiting small caves"),
            answer: num_routes_part1 as i64,
        }),
        Some(util::PartialSolution {
            message: String::from("Number paths revisiting at most one small cave"),
            answer: num_routes_part2 as i64,
        }),
    )
}
