use crate::util;
use regex::Regex;
use std::cmp;
use std::collections::VecDeque;
use std::fs;
use std::path;

type RotationMatrix3D = ((i64, i64, i64), (i64, i64, i64), (i64, i64, i64));

// rustfmt wants to format these matrices as single lines,
// which I think is far less readable
#[rustfmt::skip]
const ROTATE_X: RotationMatrix3D = (
    (1, 0,  0),
    (0, 0, -1),
    (0, 1,  0),
);
#[rustfmt::skip]
const ROTATE_Y: RotationMatrix3D = (
    ( 0, 0, 1),
    ( 0, 1, 0),
    (-1, 0, 0),
);
#[rustfmt::skip]
const ROTATE_Z: RotationMatrix3D = (
    (0, -1, 0),
    (1,  0, 0),
    (0,  0, 1),
);

type Point3D = (i64, i64, i64);

fn translate_point(p1: &Point3D, p2: &Point3D) -> Point3D {
    (p1.0 + p2.0, p1.1 + p2.1, p1.2 + p2.2)
}

fn rotate_point(point: &Point3D, rot: &RotationMatrix3D) -> Point3D {
    (
        rot.0 .0 * point.0 + rot.0 .1 * point.1 + rot.0 .2 * point.2,
        rot.1 .0 * point.0 + rot.1 .1 * point.1 + rot.1 .2 * point.2,
        rot.2 .0 * point.0 + rot.2 .1 * point.1 + rot.2 .2 * point.2,
    )
}

fn translate_points(points: &[Point3D], translation: &Point3D) -> Vec<Point3D> {
    points
        .iter()
        .map(|p| translate_point(p, translation))
        .collect()
}

fn rotate_points(points: &[Point3D], rotation: &RotationMatrix3D) -> Vec<Point3D> {
    points.iter().map(|p| rotate_point(p, rotation)).collect()
}

fn align_points(reference: &[Point3D], to_align: &[Point3D]) -> Option<(Vec<Point3D>, Point3D)> {
    let mut reference_copy = Vec::from(reference);
    reference_copy.sort_unstable();
    let mut to_align_copy = Vec::from(to_align);
    to_align_copy.sort_unstable();
    for ref_point in reference_copy.iter() {
        let ref_negated = (-ref_point.0, -ref_point.1, -ref_point.2);
        for other_point in to_align_copy.iter() {
            let mut other_copy = *other_point;
            let other_negated = (-other_point.0, -other_point.1, -other_point.2);
            let mut transformed_other = translate_points(&to_align_copy, &other_negated);
            // This is redundant since it checks some rotations multiple times
            for outer_matrix in [ROTATE_X, ROTATE_Y].iter() {
                for _i in 0..4 {
                    for _j in 0..4 {
                        transformed_other = translate_points(&transformed_other, ref_point);
                        let match_count = transformed_other
                            .iter()
                            .filter(|p| reference_copy.binary_search(p).is_ok())
                            .count();
                        if match_count >= 12 {
                            let other_origin = translate_point(
                                ref_point,
                                &(-other_copy.0, -other_copy.1, -other_copy.2),
                            );
                            let mut combined_points = reference_copy;
                            combined_points.append(&mut transformed_other);
                            combined_points.sort_unstable();
                            combined_points.dedup();
                            return Some((combined_points, other_origin));
                        }
                        transformed_other = translate_points(&transformed_other, &ref_negated);
                        transformed_other = rotate_points(&transformed_other, &ROTATE_Z);
                        other_copy = rotate_point(&other_copy, &ROTATE_Z);
                    }
                    transformed_other = rotate_points(&transformed_other, outer_matrix);
                    other_copy = rotate_point(&other_copy, outer_matrix);
                }
            }
        }
    }
    None
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let scanner_regex = Regex::new(r"^--- scanner (\d+) ---$").unwrap();
    let beacon_regex = Regex::new(r"^(?P<x>-?\d+),(?P<y>-?\d+),(?P<z>-?\d+)$").unwrap();

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut all_beacons: VecDeque<Vec<Point3D>> = VecDeque::new();
    let mut beacons: Vec<Point3D> = Vec::new();
    let mut beacon_count: i64 = 0;
    for line in contents.lines() {
        if scanner_regex.is_match(line) {
            if !beacons.is_empty() {
                all_beacons.push_back(beacons);
                beacons = Vec::new();
            }
        } else if let Some(captures) = beacon_regex.captures(line) {
            beacons.push((
                captures.name("x").unwrap().as_str().parse().unwrap(),
                captures.name("y").unwrap().as_str().parse().unwrap(),
                captures.name("z").unwrap().as_str().parse().unwrap(),
            ));
            beacon_count += 1;
        } else if !line.is_empty() {
            println!("Warning: unrecognized line: '{}'", line);
        }
    }
    if !beacons.is_empty() {
        all_beacons.push_back(beacons);
    }
    println!(
        "Found {} scanners and {} beacons",
        all_beacons.len(),
        beacon_count,
    );

    let mut aligned_beacons = all_beacons.pop_front().expect("No beacons found");
    let mut origins: Vec<Point3D> = vec![(0, 0, 0)];
    while !all_beacons.is_empty() {
        println!("{} scanners left to align", all_beacons.len());
        let to_align = all_beacons.pop_front().unwrap();
        if let Some((aligned, other_origin)) = align_points(&aligned_beacons, &to_align) {
            aligned_beacons = aligned;
            origins.push(other_origin);
        } else {
            println!("Unable to align scanner, skipping for now");
            all_beacons.push_back(to_align);
        }
    }

    let mut max_distance = 0;
    for (i, org1) in origins.iter().enumerate() {
        for org2 in origins[i + 1..].iter() {
            let distance =
                (org1.0 - org2.0).abs() + (org1.1 - org2.1).abs() + (org1.2 - org2.2).abs();
            max_distance = cmp::max(max_distance, distance);
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number of beacons"),
            answer: aligned_beacons.len() as i64,
        }),
        Some(util::PartialSolution {
            message: String::from("Maximum distance between scanners"),
            answer: max_distance,
        }),
    )
}
