use crate::util::{self, Point2D};
use std::cmp;
use std::collections::{BinaryHeap, HashMap, HashSet};
use std::fs;
use std::hash;
use std::path;

// This solution is very slow. I use Dijkstra's algorithm, which often performs
// well for AoC problems. However, since it prioritizes lower energy
// arrangements (which is why it's able to find an optimal solution), it ends
// up being almost as slow as brute-force because it vastly prefers moving As
// and Bs compared to Cs and Ds.

#[rustfmt::skip]
const PART2_EXTENSION: [((usize, usize), char); 8] = [
    ((3, 3), 'D'), ((3, 5), 'C'), ((3, 7), 'B'), ((3, 9), 'A'),
    ((4, 3), 'D'), ((4, 5), 'B'), ((4, 7), 'A'), ((4, 9), 'C')
];

#[derive(Clone, Debug)]
enum CellType {
    SideRoom(char),
    Hallway,
    ImmediatelyOutsideRoom,
}

type BurrowType = HashMap<Point2D, CellType>;
type EnergyType = HashMap<char, i64>;

#[derive(Clone, Debug)]
struct AmphipodArrangement {
    locations: HashMap<Point2D, char>,
    energy: i64,
}

impl Eq for AmphipodArrangement {}

impl hash::Hash for AmphipodArrangement {
    fn hash<H: hash::Hasher>(&self, state: &mut H) {
        let mut as_vec: Vec<_> = self.locations.iter().collect();
        as_vec.sort_unstable();
        as_vec.hash(state);
    }
}

impl Ord for AmphipodArrangement {
    fn cmp(&self, other: &Self) -> cmp::Ordering {
        other.energy.cmp(&self.energy)
    }
}

impl PartialEq for AmphipodArrangement {
    fn eq(&self, other: &Self) -> bool {
        self.locations.eq(&other.locations)
    }
}

impl PartialOrd for AmphipodArrangement {
    fn partial_cmp(&self, other: &Self) -> Option<cmp::Ordering> {
        Some(self.cmp(other))
    }
}

fn get_neighbors(point: &Point2D, burrow: &BurrowType) -> Vec<Point2D> {
    let mut neighbors: Vec<Point2D> = Vec::new();
    for row in point.0 - 1..point.0 + 2 {
        for col in point.1 - 1..point.1 + 2 {
            if (row != point.0 && col != point.1) || (row == point.0 && col == point.1) {
                continue;
            }
            if burrow.contains_key(&(row, col)) {
                neighbors.push((row, col));
            }
        }
    }
    assert!(!neighbors.is_empty());
    neighbors
}

fn get_open_neighbors(
    point: &Point2D,
    burrow: &BurrowType,
    amphipod_arrangement: &AmphipodArrangement,
    amphipod_type: char,
    movement_cost: i64,
) -> Vec<(Point2D, i64)> {
    let mut open_neighbors: Vec<(Point2D, i64)> = Vec::new();
    let source_cell = burrow.get(point).unwrap();
    for neigh_loc in get_neighbors(point, burrow) {
        if amphipod_arrangement.locations.contains_key(&neigh_loc) {
            // This cell is already occupied
            continue;
        }
        let neigh_cell = burrow.get(&neigh_loc).unwrap();
        match neigh_cell {
            CellType::Hallway => {
                open_neighbors.push((neigh_loc, movement_cost));
            }
            _ => {
                if let CellType::SideRoom(dest) = neigh_cell {
                    if matches!(source_cell, CellType::ImmediatelyOutsideRoom)
                        && *dest != amphipod_type
                    {
                        // This side-room is for a different type of amphipod,
                        // so we can't move into it.
                        continue;
                    }
                }
                // Recursively call get_open_neighbors().
                // If we're exiting a side-room, this will move us fully
                // out into the hallway so we're not blocking the room.
                // If we're entering a side-room, this will move us in
                // as far as we can go.
                let mut arrangement_copy = amphipod_arrangement.clone();
                // Pretend there's an amphipod at point to avoid
                // going backward from neigh to point.
                arrangement_copy
                    .locations
                    .entry(*point)
                    .or_insert(amphipod_type);
                let neigh_neigh = get_open_neighbors(
                    &neigh_loc,
                    burrow,
                    &arrangement_copy,
                    amphipod_type,
                    movement_cost,
                );
                if !neigh_neigh.is_empty() {
                    for (loc, cost) in neigh_neigh.into_iter() {
                        open_neighbors.push((loc, cost + movement_cost));
                    }
                } else {
                    // Can't keep going, so add this immediate neighbor instead
                    if !matches!(neigh_cell, CellType::ImmediatelyOutsideRoom) {
                        open_neighbors.push((neigh_loc, movement_cost));
                    }
                }
            }
        }
    }
    open_neighbors
}

fn reachable_amphipod_arrangements(
    amphipod_arrangement: &AmphipodArrangement,
    burrow: &BurrowType,
    energy_costs: &EnergyType,
) -> Vec<(AmphipodArrangement, i64)> {
    let mut other_arrangements: Vec<(AmphipodArrangement, i64)> = Vec::new();
    for (amph_loc, amph) in amphipod_arrangement.locations.iter() {
        for (other_loc, cost) in get_open_neighbors(
            amph_loc,
            burrow,
            amphipod_arrangement,
            *amph,
            *energy_costs.get(amph).unwrap(),
        )
        .iter()
        {
            let mut new_arrangement = AmphipodArrangement {
                locations: amphipod_arrangement.locations.clone(),
                energy: i64::MAX,
            };
            new_arrangement.locations.remove(amph_loc);
            new_arrangement.locations.insert(*other_loc, *amph);
            other_arrangements.push((new_arrangement, *cost));
        }
    }
    other_arrangements
}

fn are_amphipods_organized(
    amphipod_arrangement: &AmphipodArrangement,
    burrow: &BurrowType,
) -> bool {
    for (loc, amph) in amphipod_arrangement.locations.iter() {
        match burrow.get(loc).unwrap() {
            CellType::SideRoom(d) => {
                if d != amph {
                    return false;
                }
            }
            _ => return false,
        }
    }
    true
}

fn organize(
    burrow: &BurrowType,
    initial_amphipod_arrangement: &AmphipodArrangement,
    energy_costs: &EnergyType,
) -> AmphipodArrangement {
    // This uses Dijkstra's algorithm
    // https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
    let mut arrangement_energies: HashSet<AmphipodArrangement> =
        HashSet::from([initial_amphipod_arrangement.clone()]);
    let mut arrangement_heap: BinaryHeap<AmphipodArrangement> =
        BinaryHeap::from([initial_amphipod_arrangement.clone()]);
    while !arrangement_heap.is_empty() {
        let curr_arrangement = arrangement_heap.pop().unwrap();
        if are_amphipods_organized(&curr_arrangement, burrow) {
            return curr_arrangement;
        }
        for (mut other_arr, cost) in
            reachable_amphipod_arrangements(&curr_arrangement, burrow, energy_costs)
        {
            if !arrangement_energies.contains(&other_arr) {
                arrangement_energies.insert(AmphipodArrangement {
                    locations: other_arr.locations.clone(),
                    energy: i64::MAX,
                });
            }

            let energy_from_curr = curr_arrangement.energy + cost;
            if energy_from_curr < arrangement_energies.get(&other_arr).unwrap().energy {
                arrangement_energies.remove(&other_arr);
                other_arr.energy = energy_from_curr;
                arrangement_energies.insert(other_arr.clone());
                arrangement_heap.push(other_arr);
            }
        }
    }
    panic!("Unable to organize amphipods");
}

fn print_burrow(burrow: &BurrowType, amphipod_arrangement: &AmphipodArrangement) {
    let max_row = burrow.keys().fold(0, |maxr, pt| cmp::max(maxr, pt.0));
    let max_col = burrow.keys().fold(0, |maxc, pt| cmp::max(maxc, pt.1));
    for row in 0..max_row + 1 {
        for col in 0..max_col + 1 {
            let mut to_print = " ".to_string();
            if let Some(cell) = burrow.get(&(row, col)) {
                to_print = match cell {
                    CellType::SideRoom(chr) => chr.to_string().to_ascii_lowercase(),
                    CellType::Hallway => ".".to_string(),
                    CellType::ImmediatelyOutsideRoom => "_".to_string(),
                };
                if let Some(amph) = amphipod_arrangement.locations.get(&(row, col)) {
                    to_print = amph.to_string();
                }
            }
            print!("{}", to_print);
        }
        println!();
    }
}

fn modify_burrow_for_part2(
    burrow: &BurrowType,
    amphipod_arrangement: &AmphipodArrangement,
) -> (BurrowType, AmphipodArrangement) {
    let mut new_burrow = burrow.clone();
    for (loc, cell) in burrow.iter() {
        if let CellType::SideRoom(amph) = cell {
            new_burrow.insert((loc.0 + 2, loc.1), CellType::SideRoom(*amph));
        }
    }

    let mut new_locations: HashMap<Point2D, char> = HashMap::new();
    for (loc, amph) in amphipod_arrangement.locations.iter() {
        let mut loc_copy = *loc;
        if loc_copy.0 > 2 {
            loc_copy.0 += 2;
        }
        new_locations.insert(loc_copy, *amph);
    }
    for (loc, amph) in PART2_EXTENSION {
        assert!(!new_locations.contains_key(&loc));
        new_locations.insert(loc, amph);
    }
    let new_arrangement = AmphipodArrangement {
        locations: new_locations,
        energy: amphipod_arrangement.energy,
    };
    (new_burrow, new_arrangement)
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let energy_costs = HashMap::from([('A', 1), ('B', 10), ('C', 100), ('D', 1000)]);

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut burrow_part1: BurrowType = HashMap::new();
    let mut amphipod_arrangement_part1 = AmphipodArrangement {
        locations: HashMap::new(),
        energy: 0,
    };
    for (row, line) in contents.lines().enumerate() {
        let mut amphipod_dest = *energy_costs.keys().min().unwrap();
        for (col, chr) in line.chars().enumerate() {
            match chr {
                '.' => {
                    burrow_part1.insert((row, col), CellType::Hallway);
                }
                ' ' => (),
                '#' => (),
                _ => {
                    if energy_costs.contains_key(&chr) {
                        burrow_part1.insert((row, col), CellType::SideRoom(amphipod_dest));
                        amphipod_dest = ((amphipod_dest as u8) + 1) as char;
                        amphipod_arrangement_part1.locations.insert((row, col), chr);
                        let cell_above = burrow_part1.get_mut(&(row - 1, col)).unwrap();
                        if let CellType::Hallway = *cell_above {
                            *cell_above = CellType::ImmediatelyOutsideRoom;
                        }
                    } else {
                        println!("Unexpected character: '{}'", chr);
                    }
                }
            }
        }
    }
    assert_eq!(amphipod_arrangement_part1.energy, 0);
    println!("Initial arrangement for part 1");
    print_burrow(&burrow_part1, &amphipod_arrangement_part1);
    let organized_part1 = organize(&burrow_part1, &amphipod_arrangement_part1, &energy_costs);
    println!("Final arrangement for part 1");
    print_burrow(&burrow_part1, &organized_part1);

    let (burrow_part2, amphipod_arrangement_part2) =
        modify_burrow_for_part2(&burrow_part1, &amphipod_arrangement_part1);
    assert_eq!(amphipod_arrangement_part2.energy, 0);
    println!("Initial arrangement for part 2");
    print_burrow(&burrow_part2, &amphipod_arrangement_part2);
    let organized_part2 = organize(&burrow_part2, &amphipod_arrangement_part2, &energy_costs);
    println!("Final arrangement for part 2");
    print_burrow(&burrow_part2, &organized_part2);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Minimum energy cost to organize original arrangement"),
            answer: organized_part1.energy,
        }),
        Some(util::PartialSolution {
            message: String::from("Minimum energy cost to organize unfolded arrangement"),
            answer: organized_part2.energy,
        }),
    )
}
