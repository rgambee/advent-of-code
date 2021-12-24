use crate::util;
use std::cmp;
use std::collections::HashMap;
use std::fs;
use std::path;

const NUM_ITERATIONS_PART1: i64 = 2;
const NUM_ITERATIONS_PART2: i64 = 50;

// Unlike util::Point2D, the coordinates need to be signed
type Point2D = (i64, i64);
type ImageType = HashMap<Point2D, bool>;

#[derive(Debug)]
struct BoundingBox {
    rmin: i64,
    rmax: i64,
    cmin: i64,
    cmax: i64,
}

impl BoundingBox {
    fn new() -> BoundingBox {
        BoundingBox {
            rmin: i64::MAX,
            rmax: i64::MIN,
            cmin: i64::MAX,
            cmax: i64::MIN,
        }
    }

    fn update(&mut self, point: &Point2D) {
        self.rmin = cmp::min(self.rmin, point.0);
        self.rmax = cmp::max(self.rmax, point.0);
        self.cmin = cmp::min(self.cmin, point.1);
        self.cmax = cmp::max(self.cmax, point.1);
    }
}

fn get_enhancement_index(
    image: &ImageType,
    location: &Point2D,
    default_pixel_value: bool,
) -> usize {
    let mut index_vec: Vec<i64> = Vec::with_capacity(9);
    for row in location.0 - 1..location.0 + 2 {
        for col in location.1 - 1..location.1 + 2 {
            if !image.contains_key(&(row, col)) {
                // println!("Using default of {} for ({}, {})", default_pixel_value, row, col);
            }
            let lit = image.get(&(row, col)).unwrap_or(&default_pixel_value);
            if *lit {
                index_vec.push(1);
            } else {
                index_vec.push(0);
            }
        }
    }
    assert_eq!(index_vec.len(), 9);
    util::digit_vector_to_int(&index_vec, 2) as usize
}

fn enhance(
    image: &ImageType,
    enhancement_algorithm: &[bool],
    bbox: &BoundingBox,
    default_pixel_value: bool,
) -> (ImageType, BoundingBox) {
    let mut new_image: ImageType = HashMap::new();
    let mut new_bbox = BoundingBox::new();
    for row in bbox.rmin - 1..bbox.rmax + 2 {
        for col in bbox.cmin - 1..bbox.cmax + 2 {
            let location = (row, col);
            let index = get_enhancement_index(image, &location, default_pixel_value);
            // println!("({}, {}) index = {}, lit = {}", row, col, index, enhancement_algorithm[index]);
            new_bbox.update(&location);
            new_image.insert(location, enhancement_algorithm[index]);
        }
    }
    (new_image, new_bbox)
}

// fn print_image(image: &ImageType, bbox: &BoundingBox) {
//     for row in bbox.rmin..bbox.rmax + 1 {
//         for col in bbox.cmin..bbox.cmax + 1 {
//             if *image.get(&(row, col)).unwrap() {
//                 print!("#");
//             } else {
//                 print!(".");
//             }
//         }
//         println!();
//     }
//     println!();
// }

fn count_lit_pixels(image: &ImageType) -> i64 {
    image.values().filter(|&v| *v).count() as i64
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut line_iter = contents.lines();

    let enhancement_algorithm: Vec<bool> = line_iter
        .next()
        .expect("No image enhancement algorithm found")
        .chars()
        .map(|c| c == '#')
        .collect();
    if enhancement_algorithm.len() != 512 {
        panic!(
            "Expected length of enhancement algorithm to be 512 but found {}",
            enhancement_algorithm.len(),
        );
    }
    let mut outskirts_toggle = false;
    if enhancement_algorithm[0] {
        if !enhancement_algorithm[511] {
            println!("Infinte image outskirts will toggle");
            outskirts_toggle = true;
        } else {
            panic!("Infinite image lights up permanently");
        }
    }

    let mut image: ImageType = HashMap::new();
    let mut bbox = BoundingBox::new();
    line_iter.next();
    for (row, line) in line_iter.enumerate() {
        for (col, chr) in line.chars().enumerate() {
            let location = (row as i64, col as i64);
            bbox.update(&location);
            image.insert(location, chr == '#');
        }
    }

    let mut num_lit_pixels_part1 = 0;
    let mut num_lit_pixels_part2 = 0;
    for i in 0..cmp::max(NUM_ITERATIONS_PART1, NUM_ITERATIONS_PART2) {
        println!(
            "Number of lit pixels after {} iterations: {}",
            i,
            count_lit_pixels(&image),
        );
        let default_pixel_value = outskirts_toggle && (i % 2 == 1);
        let result = enhance(&image, &enhancement_algorithm, &bbox, default_pixel_value);
        image = result.0;
        bbox = result.1;
        if i == NUM_ITERATIONS_PART1 - 1 {
            num_lit_pixels_part1 = count_lit_pixels(&image);
        }
        if i == NUM_ITERATIONS_PART2 - 1 {
            num_lit_pixels_part2 = count_lit_pixels(&image);
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: format!(
                "Number of lit pixels after {} iterations",
                NUM_ITERATIONS_PART1,
            ),
            answer: num_lit_pixels_part1,
        }),
        Some(util::PartialSolution {
            message: format!(
                "Number of lit pixels after {} iterations",
                NUM_ITERATIONS_PART2,
            ),
            answer: num_lit_pixels_part2,
        }),
    )
}
