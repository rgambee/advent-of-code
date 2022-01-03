use std::cmp;
use std::fmt;

pub struct PartialSolution {
    pub message: String,
    pub answer: i64,
}

pub struct Solution(pub Option<PartialSolution>, pub Option<PartialSolution>);

impl fmt::Display for PartialSolution {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}: {}", self.message, self.answer)
    }
}

impl fmt::Display for Solution {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.0.is_some() {
            writeln!(f, "PART 1\n{}", self.0.as_ref().unwrap())?;
        }
        if self.1.is_some() {
            writeln!(f, "PART 2\n{}", self.1.as_ref().unwrap())?;
        }
        Ok(())
    }
}

pub fn digit_vector_to_int(v: &[i64], base: i64) -> i64 {
    v.iter().fold(0, |acc, d| acc * base + d)
}

pub type Point2D = (usize, usize);
pub type Grid2D = Vec<Vec<i64>>;
pub type GridSlice2D = [Vec<i64>];

#[rustfmt::skip]
pub fn distance(point_a: &Point2D, point_b: &Point2D) -> usize {
    cmp::max(point_a.0, point_b.0) -
    cmp::min(point_a.0, point_b.0) +
    cmp::max(point_a.1, point_b.1) -
    cmp::min(point_a.1, point_b.1)
}

pub fn at(grid: &GridSlice2D, location: Point2D) -> Option<&i64> {
    if let Some(row) = grid.get(location.0) {
        return row.get(location.1);
    }
    None
}

pub fn at_mut(grid: &mut GridSlice2D, location: Point2D) -> Option<&mut i64> {
    if let Some(row) = grid.get_mut(location.0) {
        return row.get_mut(location.1);
    }
    None
}

pub fn get_cardinal_neighbors(grid: &GridSlice2D, center: Point2D) -> Vec<Point2D> {
    get_neighbors(grid, center, false)
}

pub fn get_all_neighbors(grid: &GridSlice2D, center: Point2D) -> Vec<Point2D> {
    get_neighbors(grid, center, true)
}

fn get_neighbors(grid: &GridSlice2D, center: Point2D, include_diagonals: bool) -> Vec<Point2D> {
    let mut neighbors: Vec<Point2D> = Vec::new();
    for i in -1..2 {
        if i < 0 && center.0 == 0 {
            continue;
        }
        for j in -1..2 {
            if j < 0 && center.1 == 0 {
                continue;
            }
            let neigh = (
                ((center.0 as i64) + i) as usize,
                ((center.1 as i64) + j) as usize,
            );
            if !include_diagonals && neigh.0 != center.0 && neigh.1 != center.1 {
                continue;
            }
            if neigh != center && at(grid, neigh).is_some() {
                neighbors.push(neigh);
            }
        }
    }
    neighbors
}
