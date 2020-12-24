import {splitIntoLines, createCoord, coordToString, coordFromString} from '../util.js';
export default main24;

const EAST_REGEX = /^e/;
const SOUTHEAST_REGEX = /^se/;
const SOUTHWEST_REGEX = /^sw/;
const WEST_REGEX = /^w/;
const NORTHWEST_REGEX = /^nw/;
const NORTHEAST_REGEX = /^ne/;
const NUM_DAYS = 100;

function main24(input) {
    const lines = splitIntoLines(input);

    let blackTiles = new Set();
    for (const line of lines) {
        flipAccordingToDirections(line, blackTiles);
    }
    const blackTilesPart1 = blackTiles.size;

    for (let i = 0; i < NUM_DAYS; ++i) {
        console.log(i, blackTiles.size);
        blackTiles = flipAccordingToNeighbors(blackTiles);
    }
    const blackTilesPart2 = blackTiles.size;

    return {'Part 1': blackTilesPart1, 'Part 2': blackTilesPart2};
}

function flipAccordingToDirections(directionString, blackTiles) {
    // In a hex grid, the axes aren't orthogonal like they are in a
    // Cartesian one, but the idea of labeling each point with a pair
    // of numbers still works. In this case, the Y axis runs north-south,
    // and the X axis runs northeast to southwest.
    let x = 0;
    let y = 0;
    while (directionString.length > 0) {
        if (directionString.search(EAST_REGEX) === 0) {
            ++x;
            directionString = directionString.slice(1);
        } else if (directionString.search(SOUTHEAST_REGEX) === 0) {
            --y;
            directionString = directionString.slice(2);
        } else if (directionString.search(SOUTHWEST_REGEX) === 0) {
            --x;
            --y;
            directionString = directionString.slice(2);
        } else if (directionString.search(WEST_REGEX) === 0) {
            --x;
            directionString = directionString.slice(1);
        } else if (directionString.search(NORTHWEST_REGEX) === 0) {
            ++y;
            directionString = directionString.slice(2);
        } else if (directionString.search(NORTHEAST_REGEX) === 0) {
            ++x;
            ++y;
            directionString = directionString.slice(2);
        } else {
            console.log('No match for', directionString);
            return;
        }
    }
    const coordString = coordToString(createCoord(x, y));
    if (blackTiles.has(coordString)) {
        blackTiles.delete(coordString);
    } else {
        blackTiles.add(coordString);
    }
}

function flipAccordingToNeighbors(blackTiles) {
    const newBlackTiles = new Set();
    const seenCoords = new Set(blackTiles);
    const coordsToCheck = Array.from(blackTiles).map(
        cs => coordFromString(cs));
    while (coordsToCheck.length > 0) {
        const coord = coordsToCheck.pop();
        const coordString = coordToString(coord);
        const isBlack = blackTiles.has(coordString);
        const blackNeighbors = countBlackNeighbors(coord, blackTiles);

        if (isBlack && (blackNeighbors === 1 || blackNeighbors === 2)) {
            // This tile stays black
            newBlackTiles.add(coordString);
        } else if ((!isBlack) && blackNeighbors === 2) {
            // This white tile flips to black
            newBlackTiles.add(coordString);
        }

        if (blackNeighbors > 0) {
            // Add this tiles neighbors to the list of ones to check
            for (const neigh of getNeighbors(coord)) {
                const neighString = coordToString(neigh);
                if (!seenCoords.has(neighString)) {
                    coordsToCheck.push(neigh);
                    seenCoords.add(neighString);
                }
            }
        }
    }
    return newBlackTiles;
}

function getNeighbors(coord) {
    const [x, y] = coord;
    return [createCoord(x + 1, y),      // east
            createCoord(x, y - 1),      // southeast
            createCoord(x - 1, y - 1),  // southwest
            createCoord(x - 1, y),      // west
            createCoord(x, y + 1),      // northwest
            createCoord(x + 1, y + 1)]; // northeast
}

function countBlackNeighbors(coord, blackTiles) {
    let blackCount = 0;
    for (const neigh of getNeighbors(coord)) {
        if (blackTiles.has(coordToString(neigh))) {
            ++blackCount;
        }
    }
    return blackCount;
}
