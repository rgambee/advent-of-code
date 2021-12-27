use crate::util;
use std::collections::VecDeque;
use std::fs;
use std::path;

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

fn decode_line(s: &str) -> Instruction {
    match s {
        "inp" => Instruction::Inp,
        "add" => Instruction::Add,
        "mul" => Instruction::Mul,
        "div" => Instruction::Div,
        "mod" => Instruction::Mod,
        "eql" => Instruction::Eql,
    }
}

type RegisterType = [i64; 4];

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let line_iter = contents.lines();

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Largest acceptable model number"),
            answer: 0,
        }),
        None,
    )
}
