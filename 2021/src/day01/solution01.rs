use std::collections::VecDeque;
use std::env;
use std::fs;

fn main() {
    let args: Vec<String> = env::args().collect();
    let filename = &args[1];
    let contents = fs::read_to_string(filename).unwrap();
    let line_iter = contents.lines();
    let mut recent: VecDeque<i32> = VecDeque::new();
    let mut singlet_increase_count = 0;
    let mut triplet_increase_count = 0;
    for line in line_iter {
        recent.push_back(line.parse().unwrap());
        if recent.len() > 1 {
            if recent[recent.len() - 2] < recent[recent.len() - 1] {
                singlet_increase_count += 1;
            }
        }
        if recent.len() > 3 {
            let oldest = recent.pop_front().unwrap();
            if oldest < *recent.back().unwrap() {
                triplet_increase_count += 1;
            }
        }
    }

    println!("PART 1");
    println!("Found {} singlet increases", singlet_increase_count);
    println!();
    println!("PART 2");
    println!("Found {} triplet increases", triplet_increase_count);
}
