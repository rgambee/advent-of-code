import {splitIntoLines, createCoord, coordToString, coordFromString,
        getNeighboringCoords} from '../util.js';
export default main17;

// I'd like to use a custom class (or even an Array!) for representing
// coordinates, but there's no way to override the equality checks that
// Sets use, so every coordinate object would be considered unique.
// Instead, coordinates are saved in Sets as strings, e.g. "-1,2,3",
// so that equality checks work correctly.

const ITERATIONS = 6;

function main17(input) {
    const lines = splitIntoLines(input);
    let grid3d = new Set();
    let grid4d = new Set();
    for (let y = 0; y < lines.length; ++y) {
        for (let x = 0; x < lines[y].length; ++x) {
            if (lines[x][y] === '#') {
                grid3d.add(coordToString(createCoord(x, y, 0)));
                grid4d.add(coordToString(createCoord(x, y, 0, 0)));
            }
        }
    }

    for (let step = 0; step < ITERATIONS; ++step) {
        grid3d = oneStep(grid3d);
        grid4d = oneStep(grid4d);
        console.log('After step', step, 'there are', grid3d.size,
                    'active cubes in the 3D grid and', grid4d.size,
                    'active cubes in the 4D grid.');
    }

    return {'Part 1': grid3d.size,
            'Part 2': grid4d.size};
}

function oneStep(grid) {
    const newGrid = new Set();
    const seen = new Set(grid);
    const coordsToCheck = Array.from(grid);
    while (coordsToCheck.length > 0) {
        const coordStr = coordsToCheck.shift();
        const coord = coordFromString(coordStr);
        const active = grid.has(coordStr);
        const activeNeighbors = countActiveNeighbors(grid, coord);
        if (active && (activeNeighbors === 2 || activeNeighbors === 3)) {
            newGrid.add(coordStr);
        } else if ((!active) && activeNeighbors === 3) {
            newGrid.add(coordStr);
        }
        if (activeNeighbors > 0) {
            // Also add all of this coord's neighbors to be checked
            // (unless we've already checked them)
            for (const neigh of getNeighboringCoords(coord)) {
                const neighStr = coordToString(neigh);
                if (!seen.has(neighStr)) {
                    seen.add(neighStr);
                    coordsToCheck.push(neighStr);
                }
            }
        }
    }
    console.log('Checked', seen.size, 'coordinates');
    return newGrid;
}

function countActiveNeighbors(grid, coord) {
    let active = 0;
    for (const neigh of getNeighboringCoords(coord)) {
        if (grid.has(coordToString(neigh))) {
            ++active;
        }
    }
    return active;
}
