use crate::util::{self, Point2D};
use std::cmp;
use std::collections::{BinaryHeap, HashMap};
use std::fs;
use std::hash;
use std::path;

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
type LocationsType = HashMap<Point2D, char>;

#[derive(Clone)]
struct AmphipodArrangement<'a> {
    amphipod_locations: LocationsType,
    energy_expended: i64,
    burrow: &'a BurrowType,
    movement_costs: &'a EnergyType,
}

impl AmphipodArrangement<'_> {
    fn estimate_total_energy(&self) -> i64 {
        // Heuristic for A* algorithm
        // It assumes each amphipod only needs to reach the top-most
        // cell of its side-room, which isn't correct.
        let mut remaining_energy = 0;
        for (loc, amph) in self.amphipod_locations.iter() {
            let mut min_dist_to_dest = usize::MAX;
            // Measure the horizontal distance
            for (other_loc, cell) in self.burrow.iter() {
                if let CellType::SideRoom(a) = cell {
                    if a == amph {
                        min_dist_to_dest = cmp::min(
                            min_dist_to_dest,
                            cmp::max(loc.1, other_loc.1) - cmp::min(loc.1, other_loc.1),
                            // util::distance(loc, other_loc),
                        );
                    }
                }
            }
            // Add the vertical distance
            if min_dist_to_dest > 0 {
                min_dist_to_dest += loc.0;
            }
            let e = (min_dist_to_dest as i64) * self.movement_costs.get(amph).unwrap();
            // println!("Energy to move {} @ {:?}: {}", amph, loc, e);
            remaining_energy += e;
        }
        self.energy_expended + remaining_energy
    }

    fn to_location_vec(&self) -> Vec<(Point2D, char)> {
        let mut as_vec: Vec<_> = self.amphipod_locations.clone().into_iter().collect();
        as_vec.sort_unstable();
        as_vec
    }
}

impl Eq for AmphipodArrangement<'_> {}

impl hash::Hash for AmphipodArrangement<'_> {
    fn hash<H: hash::Hasher>(&self, state: &mut H) {
        self.to_location_vec().hash(state);
    }
}

impl Ord for AmphipodArrangement<'_> {
    fn cmp(&self, other: &Self) -> cmp::Ordering {
        other
            .estimate_total_energy()
            .cmp(&self.estimate_total_energy())
    }
}

impl PartialEq for AmphipodArrangement<'_> {
    fn eq(&self, other: &Self) -> bool {
        self.amphipod_locations.eq(&other.amphipod_locations)
    }
}

impl PartialOrd for AmphipodArrangement<'_> {
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
    amphipod_arrangement: &AmphipodArrangement,
    amphipod_type: char,
) -> Vec<(Point2D, i64)> {
    let burrow = &amphipod_arrangement.burrow;
    let movement_cost = amphipod_arrangement
        .movement_costs
        .get(&amphipod_type)
        .unwrap();
    let mut open_neighbors: Vec<(Point2D, i64)> = Vec::new();
    let source_cell = burrow.get(point).unwrap();
    // If this amphipod is in its own side room and there are only matching
    // amphipods below, don't bother moving it.
    if matches!(source_cell, CellType::SideRoom(a) if *a == amphipod_type) {
        let mut can_stay = true;
        for row in point.0 + 1.. {
            let down = (row, point.1);
            if !burrow.contains_key(&down) {
                break;
            }
            if let Some(a) = amphipod_arrangement.amphipod_locations.get(&down) {
                if *a != amphipod_type {
                    can_stay = false;
                    break;
                }
            }
        }
        if can_stay {
            let down = (point.0 + 1, point.1);
            if burrow.contains_key(&down)
                && !amphipod_arrangement.amphipod_locations.contains_key(&down)
            {
                open_neighbors.push((down, *movement_cost));
            }
            return open_neighbors;
        }
    }
    for neigh_loc in get_neighbors(point, burrow) {
        if amphipod_arrangement
            .amphipod_locations
            .contains_key(&neigh_loc)
        {
            // This cell is already occupied
            continue;
        }
        let neigh_cell = burrow.get(&neigh_loc).unwrap();
        match neigh_cell {
            CellType::Hallway => {
                open_neighbors.push((neigh_loc, *movement_cost));
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
                    .amphipod_locations
                    .entry(*point)
                    .or_insert(amphipod_type);
                let neigh_neigh = get_open_neighbors(&neigh_loc, &arrangement_copy, amphipod_type);
                if !neigh_neigh.is_empty() {
                    for (loc, cost) in neigh_neigh.into_iter() {
                        open_neighbors.push((loc, cost + *movement_cost));
                    }
                } else {
                    // Can't keep going, so add this immediate neighbor instead
                    if !matches!(neigh_cell, CellType::ImmediatelyOutsideRoom) {
                        open_neighbors.push((neigh_loc, *movement_cost));
                    }
                }
            }
        }
    }
    open_neighbors
}

fn reachable_amphipod_arrangements<'a, 'b>(
    amphipod_arrangement: &'a AmphipodArrangement,
    burrow: &'b BurrowType,
    movement_costs: &'b EnergyType,
) -> Vec<(AmphipodArrangement<'b>, i64)> {
    let mut other_arrangements: Vec<(AmphipodArrangement, i64)> = Vec::new();
    for (amph_loc, amph) in amphipod_arrangement.amphipod_locations.iter() {
        for (other_loc, cost) in get_open_neighbors(amph_loc, amphipod_arrangement, *amph).iter() {
            let mut new_arrangement = AmphipodArrangement {
                amphipod_locations: amphipod_arrangement.amphipod_locations.clone(),
                energy_expended: amphipod_arrangement.energy_expended,
                burrow,
                movement_costs,
            };
            new_arrangement.amphipod_locations.remove(amph_loc);
            new_arrangement.amphipod_locations.insert(*other_loc, *amph);
            other_arrangements.push((new_arrangement, *cost));
        }
    }
    other_arrangements
}

fn are_amphipods_organized(amphipod_arrangement: &AmphipodArrangement) -> bool {
    for (loc, amph) in amphipod_arrangement.amphipod_locations.iter() {
        match amphipod_arrangement.burrow.get(loc).unwrap() {
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

fn organize(initial_amphipod_arrangement: &AmphipodArrangement) -> i64 {
    // This uses the A* algorithm
    // https://en.wikipedia.org/wiki/A*_search_algorithm
    let mut arrangement_heap: BinaryHeap<AmphipodArrangement> =
        BinaryHeap::from([initial_amphipod_arrangement.clone()]);
    let mut arrangement_energies: HashMap<Vec<(Point2D, char)>, i64> = HashMap::from([(
        initial_amphipod_arrangement.to_location_vec(),
        initial_amphipod_arrangement.energy_expended,
    )]);
    let mut i = 0;
    while !arrangement_heap.is_empty() {
        let curr_arrangement = arrangement_heap.pop().unwrap();
        // println!(
        //     "Considering arrangement with cost {}",
        //     curr_arrangement.estimate_total_energy(),
        // );
        // println!(
        //     "{}, {}, {}",
        //     arrangement_heap.len(),
        //     arrangement_energies.len(),
        //     curr_arrangement.estimate_total_energy(),
        // );
        // print_arrangement(&curr_arrangement);
        if are_amphipods_organized(&curr_arrangement) {
            println!("Found solution in {} iterations", i);
            return curr_arrangement.energy_expended;
        }
        for (mut other_arr, cost) in reachable_amphipod_arrangements(
            &curr_arrangement,
            initial_amphipod_arrangement.burrow,
            initial_amphipod_arrangement.movement_costs,
        )
        .into_iter()
        {
            let energy_from_curr = curr_arrangement.energy_expended + cost;
            let location_vec = other_arr.to_location_vec();
            if !arrangement_energies.contains_key(&location_vec)
                || &energy_from_curr < arrangement_energies.get(&location_vec).unwrap()
            {
                other_arr.energy_expended = energy_from_curr;
                arrangement_heap.push(other_arr);
                arrangement_energies.insert(location_vec, energy_from_curr);
            }
        }
        i += 1;
    }
    panic!("Unable to organize amphipods");
}

fn print_arrangement(amphipod_arrangement: &AmphipodArrangement) {
    let burrow = amphipod_arrangement.burrow;
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
                if let Some(amph) = amphipod_arrangement.amphipod_locations.get(&(row, col)) {
                    to_print = amph.to_string();
                }
            }
            print!("{}", to_print);
        }
        println!();
    }
}

fn modify_burrow_for_part2(
    amphipod_arrangement: &AmphipodArrangement,
) -> (BurrowType, LocationsType) {
    let mut new_burrow = amphipod_arrangement.burrow.clone();
    for (loc, cell) in amphipod_arrangement.burrow.iter() {
        if let CellType::SideRoom(amph) = cell {
            new_burrow.insert((loc.0 + 2, loc.1), CellType::SideRoom(*amph));
        }
    }

    let mut new_locations: LocationsType = HashMap::new();
    for (loc, amph) in amphipod_arrangement.amphipod_locations.iter() {
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
    (new_burrow, new_locations)
}

pub fn solve(input_path: path::PathBuf) -> util::Solution {
    let energy_costs = HashMap::from([('A', 1), ('B', 10), ('C', 100), ('D', 1000)]);

    let contents = fs::read_to_string(&input_path)
        .unwrap_or_else(|_| panic!("Failed to read input file {:?}", input_path));
    let mut burrow_part1: BurrowType = HashMap::new();
    let mut amphipod_locations: LocationsType = HashMap::new();
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
                        amphipod_locations.insert((row, col), chr);
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
    let amphipod_arrangement_part1 = AmphipodArrangement {
        amphipod_locations,
        energy_expended: 0,
        burrow: &burrow_part1,
        movement_costs: &energy_costs,
    };
    let (burrow_part2, locations_part2) = modify_burrow_for_part2(&amphipod_arrangement_part1);
    let amphipod_arrangement_part2 = AmphipodArrangement {
        amphipod_locations: locations_part2,
        energy_expended: 0,
        burrow: &burrow_part2,
        movement_costs: &energy_costs,
    };

    println!("Initial arrangement for part 1");
    print_arrangement(&amphipod_arrangement_part1);
    println!(
        "Estimated energy to organize: {}",
        amphipod_arrangement_part1.estimate_total_energy(),
    );
    let energy_part1 = organize(&amphipod_arrangement_part1);
    println!("Actual energy to organize: {}", energy_part1);

    println!("Initial arrangement for part 2");
    print_arrangement(&amphipod_arrangement_part2);
    println!(
        "Estimated energy to organize: {}",
        amphipod_arrangement_part2.estimate_total_energy(),
    );
    let energy_part2 = organize(&amphipod_arrangement_part2);
    println!("Actual energy to organize: {}", energy_part2);

    util::Solution(
        Some(util::PartialSolution {
            message: String::from("Minimum energy cost to organize original arrangement"),
            answer: energy_part1,
        }),
        Some(util::PartialSolution {
            message: String::from("Minimum energy cost to organize unfolded arrangement"),
            answer: energy_part2,
        }),
    )
}
