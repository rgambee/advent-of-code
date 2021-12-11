use crate::util;
use std::collections::HashMap;
use std::fs;
use std::path;

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    // I'd like to format these map initializations vertically,
    // but rustfmt wants to condense them to fewer lines.
    let delimiter_map: HashMap<char, char> =
        HashMap::from([('(', ')'), ('[', ']'), ('{', '}'), ('<', '>')]);
    let syntax_error_score_map: HashMap<char, i64> =
        HashMap::from([(')', 3), (']', 57), ('}', 1197), ('>', 25137)]);
    let autocomplete_score_map: HashMap<char, i64> =
        HashMap::from([(')', 1), (']', 2), ('}', 3), ('>', 4)]);

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut syntax_error_score = 0;
    let mut autocomplete_scores: Vec<i64> = Vec::new();
    for line in contents.lines() {
        let mut stack: Vec<char> = Vec::new();
        let mut corrupted = false;
        for delim in line.chars() {
            if delimiter_map.contains_key(&delim) {
                // This is a left delimiter. Push it to the stack.
                stack.push(delim);
            // Otherwise, this is a right delimiter. Pop from the
            // stack and compare to see whether they match.
            } else if let Some(left_delim) = stack.pop() {
                if delimiter_map[&left_delim] != delim {
                    corrupted = true;
                    syntax_error_score += syntax_error_score_map[&delim];
                    break;
                }
            } else {
                // Line has more right delims than left
                break;
            }
        }
        if !corrupted {
            let mut autocomp_score = 0;
            while let Some(left_delim) = stack.pop() {
                let right_delim = delimiter_map[&left_delim];
                autocomp_score = autocomp_score * 5 + autocomplete_score_map[&right_delim];
            }
            autocomplete_scores.push(autocomp_score);
        }
    }
    autocomplete_scores.sort_unstable();
    let median_autocomp_score = autocomplete_scores[autocomplete_scores.len() / 2];

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Syntax error total score"),
            answer: syntax_error_score,
        }),
        Some(util::PartialSolution {
            message: String::from("Median autocomplete score"),
            answer: median_autocomp_score,
        }),
    )
}
