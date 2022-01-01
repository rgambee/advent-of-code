use crate::util;
use regex::Regex;
use std::fs;
use std::path;

const MODEL_NUMBER_LENGTH: usize = 14;
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

fn run_instruction(
    instr: &Instruction,
    registers: &mut RegisterType,
    input_stream: &mut impl Iterator<Item = i64>,
) {
    match instr {
        Instruction::Inp(a) => registers[*a] = input_stream.next().unwrap(),
        Instruction::AddLit(a, n) => registers[*a] += n,
        Instruction::AddReg(a, b) => registers[*a] += registers[*b],
        Instruction::MulLit(a, n) => registers[*a] *= n,
        Instruction::MulReg(a, b) => registers[*a] *= registers[*b],
        Instruction::DivLit(a, n) => registers[*a] /= n,
        Instruction::DivReg(a, b) => registers[*a] /= registers[*b],
        Instruction::ModLit(a, n) => registers[*a] %= n,
        Instruction::ModReg(a, b) => registers[*a] %= registers[*b],
        Instruction::EqlLit(a, n) => registers[*a] = (registers[*a] == *n) as i64,
        Instruction::EqlReg(a, b) => registers[*a] = (registers[*a] == registers[*b]) as i64,
    }
}

fn is_model_number_valid(
    instructions: &[Instruction],
    model_number: &mut impl Iterator<Item = i64>,
) -> bool {
    let mut registers = [0; 4];
    for instr in instructions.iter() {
        run_instruction(instr, &mut registers, model_number);
    }
    if model_number.next().is_some() {
        println!("Warning: model number not fully consumed");
    }
    registers[3] == 0
}

fn search_for_valid_model_number(
    instructions: &[Instruction],
    mut model_number: [i64; MODEL_NUMBER_LENGTH],
    delta: i64,
) -> i64 {
    let mut model_numbers_tried: i64 = 0;
    while !is_model_number_valid(instructions, &mut model_number.iter().copied()) {
        model_numbers_tried += 1;
        if model_numbers_tried % 10000000 == 0 {
            println!("Tried {} model numbers", model_numbers_tried);
        }
        for i in (0..model_number.len()).rev() {
            model_number[i] += delta;
            if model_number[i] == 10 {
                model_number[i] = 1;
            } else if model_number[i] == 0 {
                model_number[i] = 9;
            } else {
                break;
            }
        }
    }
    println!("Found valid model_number: {:?}", model_number);
    util::digit_vector_to_int(&model_number, 10)
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

    let max_model_number =
        search_for_valid_model_number(&instructions, [9; MODEL_NUMBER_LENGTH], -1);
    let min_model_number =
        search_for_valid_model_number(&instructions, [1; MODEL_NUMBER_LENGTH], 1);

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
