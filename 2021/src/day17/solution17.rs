use crate::util;
use regex::Regex;
use std::cmp;
use std::fs;
use std::path;

#[derive(Clone, Copy)]
struct TargetArea {
    x0: i64,
    x1: i64,
    y0: i64,
    y1: i64,
}

/*
What are the first few values for y during the probe's path?
    y1 = y0 + v0;
    y2 = y1 + v0 - 1 = y0 + 2v0 - 1;
    y3 = y2 + v0 - 2 = y0 + 3v0 - 3;
    y4 = y3 + v0 - 3 = y0 + 4v0 - 6;

Generalizing, we see that after some time t, the y coordinate is
    y(t) = y0 + t*v0 - sum(0..t-1);
         = y0 + t*v0 - t * (t - 1) / 2
         = y0 + t*v0 - (t**2 - t) / 2           (1)

We can differentiate this with respect to t and set the result
equal to 0 to find the time when y is maximized.
    dy/dt = 0 = v0 - t + 1/2
        t = v0 + 1/2                            (2)

We can rearrange equation (1) to have the initial velocity on the left.
    v0 = [(t**2 - t) / 2 + y(t) - y0] / t       (3)

In order to have an integer solution, the numerator on the right
must be a multiple of t.
    [(t**2 - t) / 2 + y(t) - y0] % t = 0        (4)

where N is an integer.
*/

fn height_vs_t(y0: i64, yvel: i64, time: i64) -> i64 {
    // This uses equation (1) above
    y0 + yvel * time - (time * time - time) / 2
}

fn find_max_height(target_area: &TargetArea) -> i64 {
    let mut max_height = 0;
    for y_targ in target_area.y0..target_area.y1 {
        for t in 1..1000 {
            // Here we check whether equation (4) is satisfied
            let t_times_yvel = (t * t - t) / 2 + y_targ;
            if t_times_yvel % t == 0 {
                // Find the corresponding initial velocity using equation (2)
                let yvel = t_times_yvel / t;
                // According to equation (2), the ideal continuous solution would be
                //      t_peak = t_times_yvel / t + 1/2
                // which isn't an integer. To find the best discrete solution,
                // we check the integers on either side of this ideal value.
                for t_peak in yvel..yvel + 2 {
                    max_height = cmp::max(height_vs_t(0, yvel, t_peak), max_height);
                }
            }
        }
    }
    max_height
}

fn intersects_target(mut target_area: TargetArea, mut xvel: i64, mut yvel: i64) -> bool {
    if xvel < 0 {
        // Flip coordinate system to make things simpler below
        target_area = TargetArea {
            x0: -target_area.x1,
            x1: -target_area.x0,
            y0: target_area.y0,
            y1: target_area.y1,
        };
        xvel = -xvel;
    }
    let mut position = (0, 0);
    while position.0 <= target_area.x1 && position.1 >= target_area.y0 {
        if position.0 >= target_area.x0 && position.1 <= target_area.y1 {
            return true;
        }
        position = (position.0 + xvel, position.1 + yvel);
        xvel = cmp::max(xvel - 1, 0);
        yvel -= 1;
    }
    false
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    // Example
    // target area: x=20..30, y=-10..-5
    let line_regex = Regex::new(
        r"^target area: x=(?P<x0>-?\d+)\.\.(?P<x1>-?\d+), y=(?P<y0>-?\d+)\.\.(?P<y1>-?\d+)",
    )
    .unwrap();

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let captures = line_regex
        .captures(&contents)
        .expect("Failed to parse target area");
    let x0: i64 = captures.name("x0").unwrap().as_str().parse().unwrap();
    let x1: i64 = captures.name("x1").unwrap().as_str().parse().unwrap();
    let y0: i64 = captures.name("y0").unwrap().as_str().parse().unwrap();
    let y1: i64 = captures.name("y1").unwrap().as_str().parse().unwrap();
    let target_area = TargetArea { x0, x1, y0, y1 };

    let max_height = find_max_height(&target_area);

    // What's the minimum initial x velocity to reach the target area?
    //      target_area.x0 = xvel + (xvel-1) + (xvel-2) + ... + 2 + 1 + 0
    //      target_area.x0 = xvel**2 / 2
    let min_x_vel = (2.0 * target_area.x0 as f64).sqrt().round() as i64;
    // For the max x and min y velocities, we naively use the far edges of the target area.
    let max_x_vel = target_area.x1;
    let min_y_vel = target_area.y0;
    // For the max y velocity, we again naively use the max height computed above.
    let max_y_vel = max_height;
    let mut velocity_count = 0;
    for xvel in min_x_vel..max_x_vel + 1 {
        for yvel in min_y_vel..max_y_vel + 1 {
            if intersects_target(target_area, xvel, yvel) {
                velocity_count += 1;
            }
        }
    }

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Maximum height of successful trajectories"),
            answer: max_height,
        }),
        Some(util::PartialSolution {
            message: String::from(
                "Number of initial velocity values which pass through target area",
            ),
            answer: velocity_count,
        }),
    )
}
