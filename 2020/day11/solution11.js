import {splitIntoLines} from '../util.js';
export default main11;

const FLOOR = '.';
const EMPTY = 'L';
const OCCUPIED = '#';

function main11(input) {
    const originalGrid = splitIntoLines(input);

    const occupiedPart1 = runUntilSteadyState([...originalGrid], 4, 1);
    const occupiedPart2 = runUntilSteadyState([...originalGrid], 5, undefined);

    return {'Part 1': occupiedPart1, 'Part 2': occupiedPart2};
}

function runUntilSteadyState(grid, occupiedToEmptyThreshold, distanceLimit) {
    let changed = true;
    while (changed) {
        [grid, changed] = updateGrid(grid, occupiedToEmptyThreshold,
                                     distanceLimit);
    }
    console.log('Steady state reached');
    let occupiedCount = 0;
    for (const row of grid) {
        occupiedCount += (row.match(new RegExp(OCCUPIED, 'g')) || []).length;
    }
    console.log('Counted', occupiedCount, 'occupied seats');
    return occupiedCount;
}

function updateGrid(grid, occupiedToEmptyThreshold, distanceLimit) {
    const newGrid = [];
    let changed = false;

    for (let r = 0; r < grid.length; ++r) {
        const row = grid[r];
        let newRow = '';
        for (let c = 0; c < row.length; ++c) {
            if (row[c] === FLOOR) {
                newRow += FLOOR;
                continue;
            }
            const occupied = countOccupied(grid, r, c, distanceLimit);
            if (row[c] === EMPTY && occupied === 0) {
                newRow += OCCUPIED;
                changed = true;
            } else if (row[c] === OCCUPIED && occupied >= occupiedToEmptyThreshold) {
                newRow += EMPTY;
                changed = true;
            } else {
                newRow += row[c];
            }
        }
        newGrid.push(newRow);
    }

    return [newGrid, changed];
}

function countOccupied(grid, row, col, distanceLimit) {
    if (distanceLimit ===  undefined) {
        distanceLimit = grid.length;
    }
    let occupied = 0;
    for (let yDir = -1; yDir <= 1; ++yDir) {
        xDirectionLoop: for (let xDir = -1; xDir <= 1; ++xDir) {
            if (yDir === 0 && xDir === 0) {
                continue;
            }
            for (let dist = 1; dist <= distanceLimit; ++dist) {
                const rowIndex = row + yDir * dist;
                if (rowIndex < 0 || rowIndex >= grid.length ) {
                    continue xDirectionLoop;
                }
                const cell = grid[rowIndex][col + xDir * dist];
                switch (cell) {
                    case FLOOR:
                        // Keep searching in this direction.
                        continue;
                    case OCCUPIED:
                        ++occupied;
                        // Fall through
                    case EMPTY:
                        // Found a seat (empty or not), so stop searching
                        // in this direction.
                    default: // eslint-disable-line no-fallthrough
                        // undefined: out of bounds, so stop searching
                        // in this direction.
                        continue xDirectionLoop;
                }
            }
        }
    }
    return occupied;
}
