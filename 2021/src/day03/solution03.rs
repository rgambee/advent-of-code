use std::convert::TryFrom;
use std::env;
use std::fs;

fn char_to_int(c: char) -> Option<i32> {
    match c {
        '0' => Some(0),
        '1' => Some(1),
        _ => None,
    }
}

fn invert_binary_vector(v: &[i32]) -> Vec<i32> {
    v.iter().map(|b| 1 - b).collect()
}

fn binary_vector_to_int(v: &[i32]) -> i32 {
    v.iter().fold(0, |acc, b| acc * 2 + b)
}

fn most_common_bits(lines: &[Vec<i32>]) -> Vec<i32> {
    let mut bit_counts: Vec<i32> = Vec::new();
    for line in lines.iter() {
        if bit_counts.is_empty() {
            bit_counts.resize(line.len(), 0);
        } else if line.len() != bit_counts.len() {
            println!(
                "Line has unexpected number of bits: {}, expected {}",
                line.len(),
                bit_counts.len()
            );
            continue;
        }
        for (i, digit) in line.iter().enumerate() {
            bit_counts[i] += digit;
        }
    }
    bit_counts
        .iter()
        .map(|c| (2 * c) / i32::try_from(lines.len()).unwrap())
        .collect()
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let filename = &args[1];
    let contents = fs::read_to_string(filename).unwrap();
    let lines: Vec<Vec<i32>> = contents
        .lines()
        .map(|l| l.chars().map(|c| char_to_int(c).unwrap()).collect())
        .collect();

    let gamma_vec = most_common_bits(&lines);
    let gamma_value = binary_vector_to_int(&gamma_vec);
    let epsilon_value = binary_vector_to_int(&invert_binary_vector(&gamma_vec));
    println!("Gamma value: {}", gamma_value);
    println!("Epsilon value: {}", epsilon_value);

    let mut oxygen_ratings = lines.clone();
    let mut co2_scrubber_ratings = lines;
    for i in 0..gamma_vec.len() {
        if oxygen_ratings.is_empty() {
            panic!("Unable to determine oxygen rating");
        } else if oxygen_ratings.len() > 1 {
            let most_common_o2 = most_common_bits(&oxygen_ratings);
            let digit = most_common_o2[i];
            oxygen_ratings = oxygen_ratings
                .into_iter()
                .filter(|r| r[i] == digit)
                .collect();
        }
        if co2_scrubber_ratings.is_empty() {
            panic!("Unable to determine CO2 scrubber rating");
        } else if oxygen_ratings.len() > 1 {
            let most_common_co2 = most_common_bits(&co2_scrubber_ratings);
            let digit = most_common_co2[i];
            co2_scrubber_ratings = co2_scrubber_ratings
                .into_iter()
                .filter(|r| r[i] != digit)
                .collect();
        }
    }
    let oxygen_value = binary_vector_to_int(&oxygen_ratings[0]);
    let co2_scrubber_value = binary_vector_to_int(&co2_scrubber_ratings[0]);
    println!("Oxygen rating: {}", oxygen_value);
    println!("CO2 scrubber rating: {}", co2_scrubber_value);

    println!("PART 1");
    println!("Power consumption: {}", gamma_value * epsilon_value);
    println!();
    println!("PART 2");
    println!("Life support rating: {}", oxygen_value * co2_scrubber_value);
}
