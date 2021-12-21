use crate::util;
use regex::Regex;
use std::cmp;
use std::fs;
use std::path;

type RotationMatrix3D = ((i64, i64, i64), (i64, i64, i64), (i64, i64, i64));

const ROTATE_X: RotationMatrix3D = (
    (1, 0, 0),
    (0, 0, -1),
    (0, 1, 0),
);
const ROTATE_Y: RotationMatrix3D = (
    (0, 0, 1),
    (0, 1, 0),
    (-1, 0, 0),
);
const ROTATE_Z: RotationMatrix3D = (
    (0, -1, 0),
    (1, 0, 0),
    (0, 0, 1),
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

fn distance2(p1: &Point3D, p2: &Point3D) -> i64 {
    // Square of distance to avoid taking root
    let d0 = p1.0 - p2.0;
    let d1 = p1.1 - p2.1;
    let d2 = p1.2 - p2.2;
    d0 * d0 + d1 * d1 + d2 * d2
}

fn calculate_centroid(points: &[Point3D]) -> Point3D {
    if points.is_empty() {
        panic!("Cannot calculate centroid of empty slice");
    }
    let mut centroid = (0, 0, 0);
    for p in points.iter() {
        centroid = translate_point(&centroid, p);
    }
    let n = points.len() as i64;
    (centroid.0 / n, centroid.1 / n, centroid.2 / n)
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

fn center_points(points: &[Point3D]) -> Vec<Point3D> {
    let centroid = calculate_centroid(points);
    translate_points(points, &(-centroid.0, -centroid.1, -centroid.2))
}

fn calculate_error(points_a: &[Point3D], points_b: &[Point3D]) -> i64 {
    let mut total_error = 0;
    for pa in points_a.iter() {
        let mut min_dist = i64::MAX;
        for pb in points_b.iter() {
            min_dist = cmp::min(distance2(pa, pb), min_dist);
        }
        total_error += min_dist;
    }
    total_error
}

fn find_nearest_point(point: &Point3D, points: &[Point3D]) -> Point3D {
    let mut min_dist2 = i64::MAX;
    let mut nearest_point = points[0];
    for pt in points.iter() {
        let dist2 = distance2(point, pt);
        if dist2 < min_dist2 {
            min_dist2 = dist2;
            nearest_point = *pt;
        }
    }
    nearest_point
}

fn align_points(reference: &[Point3D], to_align: &[Point3D]) -> Vec<Point3D> {
    println!("Aligning points");
    let mut selected_reference = to_align
        .iter()
        .map(|pt| find_nearest_point(pt, reference))
        .collect::<Vec<Point3D>>();
    selected_reference.sort_unstable();
    selected_reference.dedup();

    let centroid_ref = calculate_centroid(&selected_reference);
    let centroid_other = calculate_centroid(to_align);
    let centered_ref = translate_points(
        &selected_reference,
        &(-centroid_ref.0, -centroid_ref.1, -centroid_ref.2),
    );
    let mut centered_other = translate_points(
        to_align,
        &(-centroid_other.0, -centroid_other.1, -centroid_other.2),
    );

    let mut min_error = calculate_error(&centered_ref, &centered_other);
    println!("Unroated error: {}", min_error);
    let mut best_other = centered_other.clone();
    let mut rotated = false;
    // This is redundant since it checks some rotations multiple times
    for _i in 0..4 {
        centered_other = rotate_points(&centered_other, &ROTATE_X);
        for _j in 0..4 {
            centered_other = rotate_points(&centered_other, &ROTATE_Y);
            for _k in 0..4 {
                centered_other = rotate_points(&centered_other, &ROTATE_Z);
                let error = calculate_error(&centered_ref, &centered_other);
                // println!("Error: {}", error);
                if error < min_error {
                    min_error = error;
                    best_other = centered_other.clone();
                    rotated = true;
                }
            }
        }
    }
    if rotated {
        // Try another iteration until it converges
        println!("Recursing");
        return align_points(reference, &best_other);
    }
    // Combine points and remove duplicates
    best_other = translate_points(&best_other, &centroid_ref);
    let mut combined = Vec::from(reference);
    combined.append(&mut best_other);
    combined.sort_unstable();
    combined.dedup();
    println!(
        "Originally {} and {} beacons, now {} (change of {})",
        reference.len(),
        to_align.len(),
        combined.len(),
        reference.len() + to_align.len() - combined.len(),
    );
    combined
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let scanner_regex = Regex::new(r"^--- scanner (\d+) ---$").unwrap();
    let beacon_regex = Regex::new(r"^(?P<x>-?\d+),(?P<y>-?\d+),(?P<z>-?\d+)$").unwrap();

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut all_beacons: Vec<Vec<Point3D>> = Vec::new();
    let mut beacons: Vec<Point3D> = Vec::new();
    let mut beacon_count: i64 = 0;
    for line in contents.lines() {
        if scanner_regex.is_match(line) {
            if !beacons.is_empty() {
                all_beacons.push(beacons);
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
        all_beacons.push(beacons);
    }
    println!(
        "Found {} scanners and {} beacons",
        all_beacons.len(),
        beacon_count,
    );

    let mut beacon_iter = all_beacons.into_iter();
    let first_set = beacon_iter.next().unwrap();
    let aligned = beacon_iter.fold(first_set, |algn, pts| align_points(&algn, &pts));

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number of beacons"),
            answer: aligned.len() as i64,
        }),
        None,
    )
}
