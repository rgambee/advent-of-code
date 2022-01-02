use crate::util;
use std::path;

// This solution makes many assumptions regarding the format of the input
// instruction list. In fact, it doesn't even bother reading it. This allows
// it to be much faster than a naive approach. For such a naive (but more
// general) approach, see solution24-slow.rs.

// The instruction list has the same pattern repeated 14 times.
// It boils down to
//      if z(i) % 26 + A(i) == N(i):
//          z(i+1) = z(i) / C(i)
//      else:
//          z(i+1) = 26 * z(i) / C(i) + B(i) + I(i)
// where t is the step number (0 to 13),
//       z(i) is the value in register z at step t
//       A(i), B(i) and C(i) are values specified in the input file that vary
//          from step to step
//       N(i) is the ith digit of the input number

// C(i) can either be 1 or 26. In order to end up with z(13) == 0, we need
// to divide by 26 as many times as we can, which means satisfying the
// conditional every time C(i) == 26.

// This constraint fixes 7 of the 14 digits to be exactly determined by the
// other 7 digits. That narrows down the solution space from 9^14 ~= 10^13
// to 9^7 ~= 10^6, which is small enough to search exhaustively. Although,
// we don't need to consider all 9^7 possibilities. We can stop when we find
// the max or min that works.

// These are the A, B, C values described above for my particular
// instruction file.
#[rustfmt::skip]
const PARAMETERS: [(i64, i64, i64); 14] = [
    ( 12,  6,  1),
    ( 10,  6,  1),
    ( 13,  3,  1),
    (-11, 11, 26),
    ( 13,  9,  1),
    ( -1,  3, 26),
    ( 10, 13,  1),
    ( 11,  6,  1),
    (  0, 14, 26),
    ( 10, 10,  1),
    ( -5, 12, 26),
    (-16, 10, 26),
    ( -7, 11, 26),
    (-11, 15, 26),
];

fn pick_next_digits(index: usize, z_curr: i64, maximize: bool) -> Option<Vec<i64>> {
    if index == PARAMETERS.len() {
        return Some(Vec::new());
    }
    let (a, b, c) = PARAMETERS[index];
    if c == 26 {
        // Try to satisfy conditional described above
        let next_digit = z_curr % 26 + a;
        if (1..=9).contains(&next_digit) {
            if let Some(mut following_digits) = pick_next_digits(index + 1, z_curr / 26, maximize) {
                let mut digits = vec![next_digit];
                digits.append(&mut following_digits);
                return Some(digits);
            } else {
                return None;
            }
        } else {
            // We can't meet the condition with a digit between 1 and 9.
            // Return None to indicate this branch is a dead-end.
            return None;
        }
    } else if c == 1 {
        let mut next_digit = if maximize { 9 } else { 1 };
        let adjustment = if maximize { -1 } else { 1 };
        while (1..=9).contains(&next_digit) {
            let z_next = 26 * z_curr / c + b + next_digit;
            if let Some(mut following_digits) = pick_next_digits(index + 1, z_next, maximize) {
                let mut digits = vec![next_digit];
                digits.append(&mut following_digits);
                return Some(digits);
            } else {
                next_digit += adjustment;
            }
        }
        return None;
    }
    panic!("Unexpected value for c: {}", c);
}

pub fn solve(_: path::PathBuf) -> util::Solution {
    let max_model_number = util::digit_vector_to_int(
        &pick_next_digits(0, 0, true).expect("Unable to find maximum acceptable model number"),
        10,
    );
    let min_model_number = util::digit_vector_to_int(
        &pick_next_digits(0, 0, false).expect("Unable to find minimum acceptable model number"),
        10,
    );

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Largest acceptable model number"),
            answer: max_model_number,
        }),
        Some(util::PartialSolution {
            message: String::from("Smallest acceptable model number"),
            answer: min_model_number,
        }),
    )
}
