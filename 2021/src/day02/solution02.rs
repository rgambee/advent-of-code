use std::env;
use std::fs;
use std::iter::FromIterator;

fn main() {
    let args: Vec<String> = env::args().collect();
    let filename = &args[1];
    let contents = fs::read_to_string(filename).unwrap();
    let line_iter = contents.lines();
    let mut horizontal = 0;
    let mut depth_part1 = 0;
    let mut depth_part2 = 0;
    for line in line_iter {
        let words = Vec::from_iter(line.split_whitespace());
        if words.len() != 2 {
            println!("Failed to parse line, skipping: {}", line);
            continue;
        }
        let dir = words[0];
        let dist = words[1].parse();
        if dist.is_err() {
            println!("Failed to parse distance, skipping: {}", line);
            continue;
        }
        let dist: i32 = dist.unwrap();
        match dir {
            "forward" => {
                horizontal += dist;
                // aim is equivalent to part 1's notion of depth
                depth_part2 += depth_part1 * dist;
            }
            "up" => depth_part1 -= dist,
            "down" => depth_part1 += dist,
            _ => {
                println!("Failed to parse direction, skipping: {}", line);
                continue;
            }
        };
    }
    let part1_product = depth_part1 * horizontal;
    let part2_product = depth_part2 * horizontal;

    println!("PART 1");
    println!("Horizontal distance: {}, depth {}", horizontal, depth_part1);
    println!(
        "Product of depth and horizontal distance: {}",
        part1_product
    );
    println!();
    println!("PART 2");
    println!("Horizontal distance: {}, depth {}", horizontal, depth_part2);
    println!(
        "Product of depth and horizontal distance: {}",
        part2_product
    );
}
