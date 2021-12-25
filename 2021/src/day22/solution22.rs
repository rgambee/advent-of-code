use crate::util;
use regex::Regex;
use std::cmp;
use std::fs;
use std::path;

// For this solution, I got some inspiration from this Reddit post by u/Ezneh:
// https://www.reddit.com/r/adventofcode/comments/rmxnay/2021_day_22_part_2_d_can_anyone_help_me_find_my/

const PART1_RADIUS: i64 = 50;

#[derive(Copy, Clone, Debug)]
struct BoundingBox {
    xmin: i64,
    xmax: i64,
    ymin: i64,
    ymax: i64,
    zmin: i64,
    zmax: i64,
    on: bool,
}

impl BoundingBox {
    fn intersect(&self, other: &BoundingBox) -> Option<BoundingBox> {
        if self.xmin <= other.xmax
            && self.xmax >= other.xmin
            && self.ymin <= other.ymax
            && self.ymax >= other.ymin
            && self.zmin <= other.zmax
            && self.zmax >= other.zmin
        {
            return Some(BoundingBox {
                xmin: cmp::max(self.xmin, other.xmin),
                xmax: cmp::min(self.xmax, other.xmax),
                ymin: cmp::max(self.ymin, other.ymin),
                ymax: cmp::min(self.ymax, other.ymax),
                zmin: cmp::max(self.zmin, other.zmin),
                zmax: cmp::min(self.zmax, other.zmax),
                on: self.on,
            });
        }
        None
    }

    fn volume(&self) -> i64 {
        (self.xmax + 1 - self.xmin) * (self.ymax + 1 - self.ymin) * (self.zmax + 1 - self.zmin)
    }
}

fn add_bbox(bboxes: &[BoundingBox], new_bbox: &BoundingBox) -> Vec<BoundingBox> {
    let mut extended_bboxes = bboxes.to_owned();
    for bbox in bboxes.iter() {
        if let Some(mut inter) = new_bbox.intersect(bbox) {
            inter.on = !bbox.on;
            extended_bboxes.push(inter);
        }
    }
    if new_bbox.on {
        extended_bboxes.push(*new_bbox);
    }
    extended_bboxes
}

fn count_on_cubes(bboxes: &[BoundingBox]) -> i64 {
    let mut count = 0;
    for bbox in bboxes.iter() {
        if bbox.on {
            count += bbox.volume();
        } else {
            count -= bbox.volume();
        }
        assert!(count >= 0);
    }
    count
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    // Example:
    // on x=-48..0,y=-47..2,z=-6..48
    let line_regex = Regex::new(r"^(?P<state>on|off) x=(?P<x0>-?\d+)..(?P<x1>-?\d+),y=(?P<y0>-?\d+)..(?P<y1>-?\d+),z=(?P<z0>-?\d+)..(?P<z1>-?\d+)$").unwrap();
    let part1_volume = BoundingBox {
        xmin: -PART1_RADIUS,
        xmax: PART1_RADIUS,
        ymin: -PART1_RADIUS,
        ymax: PART1_RADIUS,
        zmin: -PART1_RADIUS,
        zmax: PART1_RADIUS,
        on: false,
    };

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut bounding_boxes_part1: Vec<BoundingBox> = Vec::new();
    let mut bounding_boxes_part2: Vec<BoundingBox> = Vec::new();
    for line in contents.lines() {
        if let Some(captures) = line_regex.captures(line) {
            let x0: i64 = captures.name("x0").unwrap().as_str().parse().unwrap();
            let x1: i64 = captures.name("x1").unwrap().as_str().parse().unwrap();
            let y0: i64 = captures.name("y0").unwrap().as_str().parse().unwrap();
            let y1: i64 = captures.name("y1").unwrap().as_str().parse().unwrap();
            let z0: i64 = captures.name("z0").unwrap().as_str().parse().unwrap();
            let z1: i64 = captures.name("z1").unwrap().as_str().parse().unwrap();
            let bbox = BoundingBox {
                xmin: cmp::min(x0, x1),
                xmax: cmp::max(x0, x1),
                ymin: cmp::min(y0, y1),
                ymax: cmp::max(y0, y1),
                zmin: cmp::min(z0, z1),
                zmax: cmp::max(z0, z1),
                on: captures.name("state").unwrap().as_str() == "on",
            };
            if let Some(inter) = bbox.intersect(&part1_volume) {
                bounding_boxes_part1 = add_bbox(&bounding_boxes_part1, &inter);
            }
            bounding_boxes_part2 = add_bbox(&bounding_boxes_part2, &bbox);
        } else if !line.is_empty() {
            println!("Unrecognized line: '{}'", line);
        }
    }

    let cubes_on_part1 = count_on_cubes(&bounding_boxes_part1);
    let cubes_on_part2 = count_on_cubes(&bounding_boxes_part2);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Number of cubes that are on in subregion"),
            answer: cubes_on_part1,
        }),
        Some(util::PartialSolution {
            message: String::from("Number of cubes that are on in total"),
            answer: cubes_on_part2,
        }),
    )
}
