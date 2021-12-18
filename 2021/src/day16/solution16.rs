use crate::util;
use std::collections::VecDeque;
use std::fs;
use std::path;

// I'd like to use iterators for this solution instead of VecDeques,
// but the compiler kept complaining about overflow errors and recursion limits.

#[derive(Debug)]
enum OperatorType {
    Sum,
    Product,
    Minimum,
    Maximum,
    GreaterThan,
    LessThan,
    EqualTo,
}

#[derive(Debug)]
enum PacketType {
    Literal {
        value: i64,
    },
    Operator {
        otype: OperatorType,
        subpackets: Vec<Packet>,
    },
}

#[derive(Debug)]
struct Packet {
    version: i64,
    ptype: PacketType,
}

fn decode_operator(num: i64) -> OperatorType {
    match num {
        0 => OperatorType::Sum,
        1 => OperatorType::Product,
        2 => OperatorType::Minimum,
        3 => OperatorType::Maximum,
        5 => OperatorType::GreaterThan,
        6 => OperatorType::LessThan,
        7 => OperatorType::EqualTo,
        _ => panic!("Unrecognized operator value {}", num),
    }
}

fn char_to_bits(c: char) -> [u8; 4] {
    let value = c.to_digit(16).unwrap();
    [
        ((value >> 3) & 1) as u8,
        ((value >> 2) & 1) as u8,
        ((value >> 1) & 1) as u8,
        (value & 1) as u8,
    ]
}

fn parse_int(bitstream: &mut VecDeque<u8>, bit_count: usize) -> Option<i64> {
    if bitstream.len() < bit_count {
        return None;
    }
    let mut digits: Vec<i64> = Vec::new();
    digits.reserve(bit_count);
    for _ in 0..bit_count {
        digits.push(bitstream.pop_front().unwrap() as i64);
    }
    Some(util::digit_vector_to_int(&digits, 2))
}

fn parse_literal_packet(bitstream: &mut VecDeque<u8>) -> Option<i64> {
    let mut done = false;
    let mut value = 0;
    while !done {
        if let Some(bit) = bitstream.pop_front() {
            done = match bit {
                0 => true,
                1 => false,
                _ => panic!("Unexpected bit"),
            };
        } else {
            return None;
        }
        if let Some(v) = parse_int(bitstream, 4) {
            value = (value << 4) + v;
        } else {
            return None;
        }
    }
    Some(value)
}

fn parse_operator_packet(bitstream: &mut VecDeque<u8>) -> Vec<Packet> {
    let mut subpackets: Vec<Packet> = Vec::new();
    match bitstream.pop_front().unwrap() {
        0 => {
            if let Some(subpacket_length) = parse_int(bitstream, 15) {
                let target_length = bitstream.len() - (subpacket_length as usize);
                while bitstream.len() > target_length {
                    subpackets.push(parse_packet(bitstream).unwrap());
                }
                if bitstream.len() != target_length {
                    panic!("Read more bits than expected for subpackets");
                }
            }
        }
        1 => {
            if let Some(num_subpackets) = parse_int(bitstream, 11) {
                for _ in 0..num_subpackets {
                    subpackets.push(parse_packet(bitstream).unwrap());
                }
            }
        }
        _ => panic!("Unexpected bit"),
    }
    subpackets
}

fn parse_packet(bitstream: &mut VecDeque<u8>) -> Option<Packet> {
    if bitstream.len() < 6 {
        return None;
    }
    let version = parse_int(bitstream, 3);
    let type_id = parse_int(bitstream, 3);
    if version.is_none() || type_id.is_none() {
        return None;
    }
    let version = version.unwrap();
    let type_id = type_id.unwrap();
    match type_id {
        4 => parse_literal_packet(bitstream).map(|value| Packet {
            version,
            ptype: PacketType::Literal { value },
        }),
        _ => {
            let subpackets = parse_operator_packet(bitstream);
            Some(Packet {
                version,
                ptype: PacketType::Operator {
                    otype: decode_operator(type_id),
                    subpackets,
                },
            })
        }
    }
}

fn add_versions(packets: &[Packet]) -> i64 {
    let mut version_sum = 0;
    for pkt in packets.iter() {
        version_sum += pkt.version;
        version_sum += match &pkt.ptype {
            PacketType::Literal { value: _ } => 0,
            PacketType::Operator {
                otype: _,
                subpackets,
            } => add_versions(subpackets),
        };
    }
    version_sum
}

fn eval_packet(packet: &Packet) -> i64 {
    match &packet.ptype {
        PacketType::Literal { value } => *value,
        PacketType::Operator { otype, subpackets } => {
            let mut subpacket_eval_iter = subpackets.iter().map(|p| eval_packet(p));
            match otype {
                OperatorType::Sum => subpacket_eval_iter.sum(),
                OperatorType::Product => subpacket_eval_iter.product(),
                OperatorType::Minimum => subpacket_eval_iter.min().unwrap(),
                OperatorType::Maximum => subpacket_eval_iter.max().unwrap(),
                _ => {
                    let sp0 = subpacket_eval_iter.next().unwrap();
                    let sp1 = subpacket_eval_iter.next().unwrap();
                    match otype {
                        OperatorType::GreaterThan => match sp0 > sp1 {
                            true => 1,
                            false => 0,
                        },
                        OperatorType::LessThan => match sp0 < sp1 {
                            true => 1,
                            false => 0,
                        },
                        OperatorType::EqualTo => match sp0 == sp1 {
                            true => 1,
                            false => 0,
                        },
                        _ => panic!("Unrecognized operator type: {:?}", otype),
                    }
                }
            }
        }
    }
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut bitstream: VecDeque<u8> = contents
        .trim_end_matches('\n')
        .chars()
        .map(char_to_bits)
        .flatten()
        .collect();

    let mut packets: Vec<Packet> = Vec::new();
    while let Some(pkt) = parse_packet(&mut bitstream) {
        packets.push(pkt);
    }
    let version_sum = add_versions(&packets);

    let packet_result = eval_packet(&packets[0]);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Sum of all packet version numbers"),
            answer: version_sum,
        }),
        Some(util::PartialSolution {
            message: String::from("Result of evaluating packet"),
            answer: packet_result,
        }),
    )
}
