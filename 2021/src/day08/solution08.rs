use crate::util;
use std::collections::HashMap;
use std::collections::HashSet;
use std::fs;
use std::path;

type Pattern = HashSet<char>;

fn decode_patterns(mut unknown_patterns: Vec<&Pattern>) -> HashMap<i64, &Pattern> {
    let mut digit_to_pattern: HashMap<i64, &Pattern> = HashMap::new();
    for pattern in unknown_patterns.iter() {
        let digit = match pattern.len() {
            2 => Some(1),
            3 => Some(7),
            4 => Some(4),
            7 => Some(8),
            _ => None,
        };
        if let Some(dig) = digit {
            digit_to_pattern.insert(dig, pattern);
        }
    }
    assert!(digit_to_pattern.len() == 4);
    for pattern in digit_to_pattern.values() {
        unknown_patterns.retain(|p| p != pattern);
    }
    assert!(unknown_patterns.len() == 6);

    // Determine pattern for 6 by seeing which pattern
    // shares exactly one segment with the pattern for 1
    let pattern_for_1 = digit_to_pattern[&1];
    let pattern_for_6 = find_intersection_with_len(&unknown_patterns, pattern_for_1, 6, 1).unwrap();
    digit_to_pattern.insert(6, pattern_for_6);
    unknown_patterns.retain(|&p| p != pattern_for_6);

    // Determine pattern for 3 by seeing which pattern
    // shares exactly two segments with the pattern for 1
    let pattern_for_3 = find_intersection_with_len(&unknown_patterns, pattern_for_1, 5, 2).unwrap();
    digit_to_pattern.insert(3, pattern_for_3);
    unknown_patterns.retain(|&p| p != pattern_for_3);

    // Determine pattern for 2 by seeing which pattern
    // shares exactly two segments with the pattern for 4
    let pattern_for_4 = digit_to_pattern[&4];
    let pattern_for_2 = find_intersection_with_len(&unknown_patterns, pattern_for_4, 5, 2).unwrap();
    digit_to_pattern.insert(2, pattern_for_2);
    unknown_patterns.retain(|&p| p != pattern_for_2);

    // Determine pattern for 9 by seeing which pattern
    // shares exactly four segments with the pattern for 4
    let pattern_for_9 = find_intersection_with_len(&unknown_patterns, pattern_for_4, 6, 4).unwrap();
    digit_to_pattern.insert(9, pattern_for_9);
    unknown_patterns.retain(|&p| p != pattern_for_9);

    // Determine pattern for 0 by seeing which pattern
    // shares exactly three segments with the pattern for 4
    let pattern_for_0 = find_intersection_with_len(&unknown_patterns, pattern_for_4, 6, 3).unwrap();
    digit_to_pattern.insert(0, pattern_for_0);
    unknown_patterns.retain(|&p| p != pattern_for_0);

    // 5 is the remaining pattern
    assert!(unknown_patterns.len() == 1);
    let pattern_for_5 = unknown_patterns[0];
    digit_to_pattern.insert(5, pattern_for_5);

    digit_to_pattern
}

fn find_intersection_with_len<'a>(
    patterns: &[&'a Pattern],
    reference: &Pattern,
    pattern_len: usize,
    intersection_len: usize,
) -> Option<&'a Pattern> {
    for pattern in patterns.iter() {
        if pattern.len() == pattern_len
            && pattern.intersection(reference).count() == intersection_len
        {
            return Some(pattern);
        }
    }
    None
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut count_1478 = 0;
    let mut output_sum = 0;
    for line in contents.lines() {
        let mut halves = line.split('|');
        let patterns: Vec<Pattern> = halves
            .next()
            .unwrap()
            .trim_end_matches(' ')
            .split(' ')
            .map(|s| s.chars().collect())
            .collect();
        let outputs: Vec<Pattern> = halves
            .next()
            .unwrap()
            .split(' ')
            .map(|s| s.chars().collect())
            .collect();
        for value in &outputs {
            if value.len() == 2 || value.len() == 3 || value.len() == 4 || value.len() == 7 {
                count_1478 += 1;
            }
        }
        let decoded = decode_patterns(patterns.iter().collect());
        let mut output_value = 0;
        for scrambled_output in &outputs {
            for (digit, pattern) in &decoded {
                if *pattern == scrambled_output {
                    output_value = output_value * 10 + digit;
                    break;
                }
            }
        }
        println!("Output value is {}", output_value);
        output_sum += output_value
    }

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number of 1s, 4s, 7s and 8s"),
            answer: count_1478,
        }),
        Some(util::PartialSolution {
            message: String::from("Sum of all output values"),
            answer: output_sum,
        }),
    )
}
