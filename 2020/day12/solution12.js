import {splitIntoLines} from '../util.js';
export default main12;

const NORTH = 'N';
const SOUTH = 'S';
const EAST = 'E';
const WEST = 'W';
const LEFT = 'L';
const RIGHT = 'R';
const FORWARD = 'F';

const TURN_REGEX = new RegExp(`^(?<direction>${LEFT}|${RIGHT})(?<degrees>[0-9]+)$`);
const MOVE_REGEX = new RegExp(`^(?<direction>${NORTH}|${WEST}|${SOUTH}|${EAST}|${FORWARD})(?<distance>[0-9]+)$`);

function main12(input) {
    const lines = splitIntoLines(input);

    let ship = {
        x: 0,
        y: 0,
        facing: EAST
    };
    navigate(lines, ship);
    const distancePart1 = Math.abs(ship.x) + Math.abs(ship.y);

    ship = {
        x: 0,
        y: 0,
        facing: EAST
    };
    const waypoint = {
        x: 10,
        y: 1
    };
    navigate(lines, ship, waypoint);
    const distancePart2 = Math.abs(ship.x) + Math.abs(ship.y);

    return {'Part 1': distancePart1, 'Part 2': distancePart2};
}

function navigate(lines, ship, waypoint) {
    let withWaypoint = true;
    if (waypoint === undefined) {
        withWaypoint = false;
    }
    for (const line of lines) {
        let match = line.match(TURN_REGEX);
        if (match !== null) {
            if (withWaypoint) {
                rotateWaypoint(waypoint, match.groups.direction,
                               Number(match.groups.degrees));
            } else {
                rotateShip(ship, match.groups.direction,
                           Number(match.groups.degrees));
            }
            continue;
        }
        match = line.match(MOVE_REGEX);
        if (match !== null) {
            if (withWaypoint) {
                moveWaypoint(ship, waypoint, match.groups.direction,
                             Number(match.groups.distance));
            } else {
                movePoint(ship, match.groups.direction,
                          Number(match.groups.distance));
            }
            continue;
        }
        console.log('Invalid command', line);
    }
}

function rotateShip(ship, direction, degrees) {
    // Convert all rotations to LEFT (counter-clockwise)
    if (direction === RIGHT) {
        degrees = 360 - degrees;
    }
    while (degrees > 0) {
        switch (ship.facing) {
            case NORTH:
                ship.facing = WEST;
                break;
            case WEST:
                ship.facing = SOUTH;
                break;
            case SOUTH:
                ship.facing = EAST;
                break;
            case EAST:
                ship.facing = NORTH;
                break;
            default:
                console.log('Invalid rotation direction', direction);
        }
        degrees -= 90;
    }
}

function movePoint(point, direction, distance) {
    if (direction === FORWARD) {
        direction = point.facing;
    }
    switch (direction) {
        case NORTH:
            point.y += distance;
            break;
        case WEST:
            point.x -= distance;
            break;
        case SOUTH:
            point.y -= distance;
            break;
        case EAST:
            point.x += distance;
            break;
    }
}

function rotateWaypoint(waypoint, direction, degrees) {
    // Convert all rotations to LEFT (counter-clockwise)
    if (direction === RIGHT) {
        degrees = 360 - degrees;
    }
    while (degrees > 0) {
        const newX = -waypoint.y;
        const newY = waypoint.x;
        waypoint.x = newX;
        waypoint.y = newY;
        degrees -= 90;
    }
}

function moveWaypoint(ship, waypoint, direction, distance) {
    if (direction === FORWARD) {
        ship.x += waypoint.x * distance;
        ship.y += waypoint.y * distance;
    } else {
        movePoint(waypoint, direction, distance);
    }
}
