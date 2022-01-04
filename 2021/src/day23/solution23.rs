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
        // This assumes amphipods can move through each other, which isn't
        // correct but still satisfactory since A* requires that the heuristic
        // doesn't overesimate the true cost.
        let mut amphs_in_rooms: HashMap<char, i64> = HashMap::new();
        let mut remaining_energy = 0;
        for (loc, amph) in self.amphipod_locations.iter() {
            let mut min_dist_to_dest = usize::MAX;
            let mut curr_row = loc.0 as i64;
            // Measure the horizontal distance
            for (other_loc, cell) in self.burrow.iter() {
                if let CellType::SideRoom(a) = cell {
                    if a == amph {
                        min_dist_to_dest = cmp::min(
                            min_dist_to_dest,
                            cmp::max(loc.1, other_loc.1) - cmp::min(loc.1, other_loc.1),
                        );
                    }
                }
            }
            if min_dist_to_dest > 0 {
                // Add the vertical distance to get out of the room
                min_dist_to_dest += loc.0 - 1;
                // Set curr_row to 1 to indicate that this amphipod needs to
                // move into the hallway.
                curr_row = 1;
            } else {
                // If this amphipod is already in its own side-room, check
                // whether there are other amphipod types below which require
                // it to move.
                assert!(
                    matches!(self.burrow.get(loc).unwrap(), CellType::SideRoom(a) if a == amph),
                );
                let amphipods_below = get_blocked_amphipods_in_sideroom(self, amph, loc);
                let can_stay = amphipods_below.iter().all(|a| a == amph);
                if !can_stay {
                    // This amphipod needs to move into the hallway, which
                    // requires loc.0 - 1 vertical moves. Then there are at
                    // least 2 horizontal moves needed: 1 to step to the side
                    // so that the room is unblocked and 1 more to go back.
                    min_dist_to_dest = loc.0 + 1;
                    // As above, set curr_row to 1 to indicate that this
                    // amphipod needs to move into the hallway.
                    curr_row = 1;
                }
            }
            // Cast this to a signed number since it can be negative below;
            let mut min_dist_to_dest = min_dist_to_dest as i64;
            // Add the distance to move into the room (or move farther in if
            // already there)
            let count = amphs_in_rooms.entry(*amph).or_insert(0);
            let final_row = get_sideroom_locations(self.burrow, amph).len() as i64 - *count + 1;
            // This difference can be negative if this amphipod is already in
            // its side-room, but we've already considered other amphipods of
            // the same type. We've assumed that those other amphipods need to
            // move into the room farther than they need to, and a negative
            // difference here will correct for that.
            min_dist_to_dest += final_row - curr_row;
            *count += 1;

            let e = min_dist_to_dest * self.movement_costs.get(amph).unwrap();
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

fn get_sideroom_locations(burrow: &BurrowType, room: &char) -> Vec<Point2D> {
    let mut locations: Vec<Point2D> = Vec::new();
    for (loc, cell) in burrow.iter() {
        if matches!(cell, CellType::SideRoom(r) if r == room) {
            locations.push(*loc);
        }
    }
    locations
}

fn get_blocked_amphipods_in_sideroom(
    arrangement: &AmphipodArrangement,
    room: &char,
    blocker_location: &Point2D,
) -> Vec<char> {
    let mut amphipods: Vec<char> = Vec::new();
    for loc in get_sideroom_locations(arrangement.burrow, room) {
        if loc.0 > blocker_location.0 {
            if let Some(amph) = arrangement.amphipod_locations.get(&loc) {
                amphipods.push(*amph);
            }
        }
    }
    amphipods
}

fn get_lowest_open_sideroom_location(
    arrangement: &AmphipodArrangement,
    room: &char,
) -> Option<Point2D> {
    let mut locs = get_sideroom_locations(arrangement.burrow, room);
    locs.sort_unstable();
    let mut lowest_loc = None;
    for loc in locs.into_iter() {
        if !arrangement.amphipod_locations.contains_key(&loc) {
            lowest_loc = Some(loc);
        } else {
            break;
        }
    }
    lowest_loc
}

fn get_open_hallway_locations(
    arrangement: &AmphipodArrangement,
    location: &Point2D,
    include_cells_immediately_outside_rooms: bool,
) -> Vec<Point2D> {
    let mut open_locations: Vec<Point2D> = Vec::new();
    for step in [-1, 1] {
        let mut loc: Point2D = (location.0, (location.1 as i64 + step).try_into().unwrap());
        while arrangement.burrow.contains_key(&loc)
            && !arrangement.amphipod_locations.contains_key(&loc)
        {
            if matches!(arrangement.burrow.get(&loc).unwrap(), CellType::Hallway)
                || include_cells_immediately_outside_rooms
            {
                open_locations.push(loc);
            }
            loc = (location.0, (loc.1 as i64 + step).try_into().unwrap());
        }
    }
    open_locations
}

fn get_possible_moves(
    location: &Point2D,
    amphipod_arrangement: &AmphipodArrangement,
    amphipod_type: &char,
) -> Vec<(Point2D, i64)> {
    let burrow = &amphipod_arrangement.burrow;
    let movement_cost = amphipod_arrangement
        .movement_costs
        .get(amphipod_type)
        .unwrap();
    let mut open_locations: Vec<(Point2D, i64)> = Vec::new();
    let cell = burrow.get(location).unwrap();
    match cell {
        CellType::SideRoom(src) => {
            let cell_above_is_open = !amphipod_arrangement
                .amphipod_locations
                .contains_key(&(location.0 - 1, location.1));
            let needs_to_move_out = src != amphipod_type
                || get_blocked_amphipods_in_sideroom(amphipod_arrangement, amphipod_type, location)
                    .into_iter()
                    .any(|a| &a != amphipod_type);
            if cell_above_is_open && needs_to_move_out {
                // This amphipod needs to move out into the hallway, either
                // because it's in the wrong room or because it's blocking
                // another amphipod that needs to move out.
                let mut loc_immediately_outside_this_room = (location.0 - 1, location.1);
                while !matches!(
                    burrow.get(&loc_immediately_outside_this_room).unwrap(),
                    CellType::ImmediatelyOutsideRoom,
                ) {
                    loc_immediately_outside_this_room =
                        (loc_immediately_outside_this_room.0 - 1, location.1);
                }
                for hallway_loc in get_open_hallway_locations(
                    amphipod_arrangement,
                    &loc_immediately_outside_this_room,
                    false,
                ) {
                    let dist = util::distance(location, &hallway_loc) as i64;
                    open_locations.push((hallway_loc, dist * movement_cost));
                }
            }
        }
        CellType::Hallway => {
            // Move into sideroom if possible
            if let Some(room_loc) =
                get_lowest_open_sideroom_location(amphipod_arrangement, amphipod_type)
            {
                // The room has an open spot
                if get_blocked_amphipods_in_sideroom(amphipod_arrangement, amphipod_type, &room_loc)
                    .into_iter()
                    .all(|a| &a == amphipod_type)
                {
                    // There aren't any other amphipod types in the room
                    let hallway_locs =
                        get_open_hallway_locations(amphipod_arrangement, location, true);
                    if hallway_locs.into_iter().any(|loc| loc.1 == room_loc.1) {
                        // There's a free path to the room
                        let dist = util::distance(location, &room_loc) as i64;
                        open_locations.push((room_loc, dist * movement_cost))
                    }
                }
            }
        }
        CellType::ImmediatelyOutsideRoom => {
            panic!("Amphipod can't stop immediately outside room");
        }
    }
    open_locations
}

fn reachable_amphipod_arrangements<'a, 'b>(
    amphipod_arrangement: &'a AmphipodArrangement<'b>,
    burrow: &'b BurrowType,
    movement_costs: &'b EnergyType,
) -> Vec<AmphipodArrangement<'b>> {
    let mut other_arrangements: Vec<AmphipodArrangement> = Vec::new();
    for (amph_loc, amph) in amphipod_arrangement.amphipod_locations.iter() {
        for (other_loc, cost) in
            get_possible_moves(amph_loc, amphipod_arrangement, amph).into_iter()
        {
            let mut new_arrangement = AmphipodArrangement {
                amphipod_locations: amphipod_arrangement.amphipod_locations.clone(),
                energy_expended: amphipod_arrangement.energy_expended + cost,
                burrow,
                movement_costs,
            };
            new_arrangement.amphipod_locations.remove(amph_loc);
            new_arrangement.amphipod_locations.insert(other_loc, *amph);
            other_arrangements.push(new_arrangement);
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
        if are_amphipods_organized(&curr_arrangement) {
            println!("Found solution in {} iterations", i);
            print_arrangement(&curr_arrangement);
            return curr_arrangement.energy_expended;
        }
        for other_arr in reachable_amphipod_arrangements(
            &curr_arrangement,
            initial_amphipod_arrangement.burrow,
            initial_amphipod_arrangement.movement_costs,
        )
        .into_iter()
        {
            let location_vec = other_arr.to_location_vec();
            if !arrangement_energies.contains_key(&location_vec)
                || &other_arr.energy_expended < arrangement_energies.get(&location_vec).unwrap()
            {
                arrangement_energies.insert(location_vec, other_arr.energy_expended);
                arrangement_heap.push(other_arr);
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
