use crate::util;
use regex::Regex;
use std::cmp;
use std::collections::HashMap;
use std::fs;
use std::path;

const NUM_STEPS_PART1: i64 = 10;
const NUM_STEPS_PART2: i64 = 40;

// I feel like the lifetimes for this function aren't quite correct,
// but this is the best I've found which manages to compile.
fn react<'a, 'b>(
    polymer_pairs: &'a HashMap<&'b str, i64>,
    insertion_rules: &'b HashMap<&str, (String, String)>,
) -> HashMap<&'b str, i64> {
    let mut new_pairs: HashMap<&str, i64> = HashMap::new();
    for (input_pair, count) in polymer_pairs {
        if let Some(output_pairs) = insertion_rules.get(input_pair) {
            let pair_a_count = new_pairs.entry(&output_pairs.0).or_insert(0);
            *pair_a_count += count;
            let pair_b_count = new_pairs.entry(&output_pairs.1).or_insert(0);
            *pair_b_count += count;
        } else {
            println!("No rule for pair {}", input_pair);
        }
    }
    new_pairs
}

fn calculate_element_difference(polymer_pairs: &HashMap<&str, i64>) -> i64 {
    let mut element_counts: HashMap<char, i64> = HashMap::new();
    for (pair, pair_count) in polymer_pairs {
        for c in pair.chars() {
            let elem_count = element_counts.entry(c).or_insert(0);
            *elem_count += pair_count;
        }
    }
    // The loop above double counts (almost) all the elements.
    // For instance, 'ABC' contains pairs 'AB' and 'BC', which would
    // lead to 'B' having a count of 2. We need to divide each count
    // by 2 to correct for this. Odd numbers need to be rounded up since
    // the correspond to elements at the beginning and end of the string,
    // which weren't double-counted.
    for val in element_counts.values_mut() {
        *val = (*val as f64 / 2.0).ceil() as i64;
    }

    let element_difference = element_counts.values().max().expect("No maximum element)")
        - element_counts.values().min().expect("No minimum element");
    element_difference
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    // Example:
    // AB -> C
    let rule_regex =
        Regex::new(r"^(?P<input_pair>[A-Z]{2}) -> (?P<output_element>[A-Z])$").unwrap();

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut line_iter = contents.lines();
    let polymer_template = line_iter.next().expect("No polymer template present");
    let mut polymer_pairs: HashMap<&str, i64> = HashMap::new();
    for i in 0..polymer_template.len() - 1 {
        let pair = &polymer_template[i..i + 2];
        let count = polymer_pairs.entry(pair).or_insert(0);
        *count += 1;
    }

    let mut insertion_rules: HashMap<&str, (String, String)> = HashMap::new();
    for line in line_iter {
        if let Some(captures) = rule_regex.captures(line) {
            let input_pair = captures.name("input_pair").unwrap().as_str();
            let mut input_pair_iter = input_pair.chars();
            let output_elem = captures.name("output_element").unwrap().as_str();
            let mut output_pairs = (String::new(), String::new());
            output_pairs.0.push(input_pair_iter.next().unwrap());
            output_pairs.0.push_str(output_elem);
            output_pairs.1.push_str(output_elem);
            output_pairs.1.push(input_pair_iter.next().unwrap());
            insertion_rules.insert(input_pair, output_pairs);
        }
    }

    let mut element_difference_part1 = -1;
    let mut element_difference_part2 = -1;
    for i in 0..cmp::max(NUM_STEPS_PART1, NUM_STEPS_PART2) {
        polymer_pairs = react(&polymer_pairs, &insertion_rules);
        if i == NUM_STEPS_PART1 - 1 {
            element_difference_part1 = calculate_element_difference(&polymer_pairs);
        }
        if i == NUM_STEPS_PART2 - 1 {
            element_difference_part2 = calculate_element_difference(&polymer_pairs);
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: format!(
                "Difference between most and least common elements after {} steps",
                NUM_STEPS_PART1
            ),
            answer: element_difference_part1,
        }),
        Some(util::PartialSolution {
            message: format!(
                "Difference between most and least common elements after {} steps",
                NUM_STEPS_PART2
            ),
            answer: element_difference_part2,
        }),
    )
}
