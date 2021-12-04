use std::env;
use std::path;

mod day01;
mod day02;
mod day03;
mod util;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        panic!("Usage: {} DAY_NUMBER", args[0]);
    }
    let day_number: i32 = args[1]
        .parse()
        .unwrap_or_else(|_| panic!("Usage: {} DAY_NUMBER", args[0]));
    let input_path: path::PathBuf = [
        format!("day{:02}", day_number),
        format!("input{:02}.txt", day_number),
    ]
    .iter()
    .collect();
    let solution = match day_number {
        1 => day01::solve(input_path),
        2 => day02::solve(input_path),
        3 => day03::solve(input_path),
        _ => panic!("Invalid day number {}", day_number),
    };
    print!("{}", solution);
}
