use std::env;
use std::path;

mod day01;
mod day02;
mod day03;
mod day04;
mod day05;
mod day06;
mod day07;
mod day08;
mod day09;
mod day10;
mod day11;
mod day12;
mod day13;
mod day14;
mod day15;
mod day16;
mod day17;
mod day18;
mod day19;
mod day20;
mod day21;
mod day22;
mod day23;
mod day24;
mod day25;
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
        String::from("src"),
        format!("day{:02}", day_number),
        format!("input{:02}.txt", day_number),
    ]
    .iter()
    .collect();
    let solution = match day_number {
        1 => day01::solve(input_path),
        2 => day02::solve(input_path),
        3 => day03::solve(input_path),
        4 => day04::solve(input_path),
        5 => day05::solve(input_path),
        6 => day06::solve(input_path),
        7 => day07::solve(input_path),
        8 => day08::solve(input_path),
        9 => day09::solve(input_path),
        10 => day10::solve(input_path),
        11 => day11::solve(input_path),
        12 => day12::solve(input_path),
        13 => day13::solve(input_path),
        14 => day14::solve(input_path),
        15 => day15::solve(input_path),
        16 => day16::solve(input_path),
        17 => day17::solve(input_path),
        18 => day18::solve(input_path),
        19 => day19::solve(input_path),
        20 => day20::solve(input_path),
        21 => day21::solve(input_path),
        22 => day22::solve(input_path),
        23 => day23::solve(input_path),
        24 => day24::solve(input_path),
        25 => day25::solve(input_path),
        _ => panic!("Invalid day number {}", day_number),
    };
    print!("{}", solution);
}
