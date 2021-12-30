use crate::util;
use regex::Regex;
use std::fs;
use std::path;

type RegisterType = [i64; 4];

enum Instruction {
    Inp(usize),
    AddLit(usize, i64),
    AddReg(usize, usize),
    MulLit(usize, i64),
    MulReg(usize, usize),
    DivLit(usize, i64),
    DivReg(usize, usize),
    ModLit(usize, i64),
    ModReg(usize, usize),
    EqlLit(usize, i64),
    EqlReg(usize, usize),
}

fn run_instruction(
    instr: Instruction,
    registers: &mut RegisterType,
    input_stream: &mut impl Iterator<Item = i64>,
) {
    match instr {
        Instruction::Inp(a) => registers[a] = input_stream.next().unwrap(),
        Instruction::AddLit(a, n) => registers[a] += n,
        Instruction::AddReg(a, b) => registers[a] += registers[b],
        Instruction::MulLit(a, n) => registers[a] *= n,
        Instruction::MulReg(a, b) => registers[a] *= registers[b],
        Instruction::DivLit(a, n) => registers[a] /= n,
        Instruction::DivReg(a, b) => registers[a] /= registers[b],
        Instruction::ModLit(a, n) => registers[a] %= n,
        Instruction::ModReg(a, b) => registers[a] %= registers[b],
        Instruction::EqlLit(a, n) => registers[a] = (registers[a] == n) as i64,
        Instruction::EqlReg(a, b) => registers[a] += (registers[a] == registers[b]) as i64,
    }
}

fn l2i(s: &str) -> usize {
    match s {
        "w" => 0,
        "x" => 1,
        "y" => 2,
        "z" => 3,
        _ => panic!("Unexpected letter {}", s),
    }
}

fn decode_line(s: &str, regexes: &[Regex; 11]) -> Instruction {
    if let Some(cap) = regexes[0].captures(s) {
        return Instruction::Inp(l2i(cap.name("a").unwrap().as_str()));
    }
    if let Some(cap) = regexes[1].captures(s) {
        return Instruction::AddLit(
            l2i(cap.name("a").unwrap().as_str()),
            cap.name("n").unwrap().as_str().parse().unwrap(),
        );
    }
    if let Some(cap) = regexes[2].captures(s) {
        return Instruction::AddReg(
            l2i(cap.name("a").unwrap().as_str()),
            l2i(cap.name("b").unwrap().as_str()),
        );
    }
    if let Some(cap) = regexes[3].captures(s) {
        return Instruction::MulLit(
            l2i(cap.name("a").unwrap().as_str()),
            cap.name("n").unwrap().as_str().parse().unwrap(),
        );
    }
    if let Some(cap) = regexes[4].captures(s) {
        return Instruction::MulReg(
            l2i(cap.name("a").unwrap().as_str()),
            l2i(cap.name("b").unwrap().as_str()),
        );
    }
    if let Some(cap) = regexes[5].captures(s) {
        return Instruction::DivLit(
            l2i(cap.name("a").unwrap().as_str()),
            cap.name("n").unwrap().as_str().parse().unwrap(),
        );
    }
    if let Some(cap) = regexes[6].captures(s) {
        return Instruction::DivReg(
            l2i(cap.name("a").unwrap().as_str()),
            l2i(cap.name("b").unwrap().as_str()),
        );
    }
    if let Some(cap) = regexes[7].captures(s) {
        return Instruction::ModLit(
            l2i(cap.name("a").unwrap().as_str()),
            cap.name("n").unwrap().as_str().parse().unwrap(),
        );
    }
    if let Some(cap) = regexes[8].captures(s) {
        return Instruction::ModReg(
            l2i(cap.name("a").unwrap().as_str()),
            l2i(cap.name("b").unwrap().as_str()),
        );
    }
    if let Some(cap) = regexes[9].captures(s) {
        return Instruction::EqlLit(
            l2i(cap.name("a").unwrap().as_str()),
            cap.name("n").unwrap().as_str().parse().unwrap(),
        );
    }
    if let Some(cap) = regexes[10].captures(s) {
        return Instruction::EqlReg(
            l2i(cap.name("a").unwrap().as_str()),
            l2i(cap.name("b").unwrap().as_str()),
        );
    }
    panic!("Could not parse {}", s);
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let instruction_regexes = [
        // Example: inp w
        Regex::new(r"^inp (?P<a>[w-z])$").unwrap(),
        // Example: add x 12
        Regex::new(r"^add (?P<a>[w-z]) (?P<n>-?\d+)$").unwrap(),
        // Example: add x z
        Regex::new(r"^add (?P<a>[w-z]) (?P<b>[w-z])$").unwrap(),
        // Example: mul x 0
        Regex::new(r"^mul (?P<a>[w-z]) (?P<n>-?\d+)$").unwrap(),
        // Example: mul y x
        Regex::new(r"^mul (?P<a>[w-z]) (?P<b>[w-z])$").unwrap(),
        // Example: div z 1
        Regex::new(r"^div (?P<a>[w-z]) (?P<n>-?\d+)$").unwrap(),
        // Example: div w z
        Regex::new(r"^div (?P<a>[w-z]) (?P<b>[w-z])$").unwrap(),
        // Example: mod z 26
        Regex::new(r"^mod (?P<a>[w-z]) (?P<n>-?\d+)$").unwrap(),
        // Example: mod x y
        Regex::new(r"^mod (?P<a>[w-z]) (?P<b>[w-z])$").unwrap(),
        // Example: eql x 0
        Regex::new(r"^eql (?P<a>[w-z]) (?P<n>-?\d+)$").unwrap(),
        // Example: eql x w
        Regex::new(r"^eql (?P<a>[w-z]) (?P<b>[w-z])$").unwrap(),
    ];

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let instructions: Vec<Instruction> = contents
        .lines()
        .map(|s| decode_line(s, &instruction_regexes))
        .collect();
    println!("Parsed {} instructions", instructions.len());

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Largest acceptable model number"),
            answer: 0,
        }),
        None,
    )
}
