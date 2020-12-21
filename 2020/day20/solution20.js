import {splitIntoLines, reverseString, createCoord, coordToString,
        coordFromString} from '../util.js';
export default main20;

// Note: I mistakenly convinced myself that just because two tiles COULD
// fit together didn't mean they SHOULD. So my algorithm for solving the
// puzzle is quite a bit more complicated than it needs to be since it
// keeps looking for other insertion spots after it's found one.

const TILE_ID_REGEX = /^Tile (?<id>[0-9]+):/;
const MATCHING_EDGES = new Map([
    ['topEdge', 'bottomEdge'],
    ['rightEdge', 'leftEdge'],
    ['bottomEdge', 'topEdge'],
    ['leftEdge', 'rightEdge']
]);
const SEA_MONSTER_TEMPLATE = `                  # 
#    ##    ##    ###
 #  #  #  #  #  #   `;

class Tile {
    constructor(edges, id, interior) {
        [this.topEdge, this.rightEdge, this.bottomEdge, this.leftEdge] = edges;
        this.id = id;
        this.interior = interior;
    }

    allEdges() {
        return [this.topEdge, this.rightEdge, this.bottomEdge, this.leftEdge];
    }

    flipUpDown() {
        [this.topEdge, this.bottomEdge] = [this.bottomEdge, this.topEdge];
        this.rightEdge = reverseString(this.rightEdge);
        this.leftEdge = reverseString(this.leftEdge);
        this.interior.reverse();
    }

    flipLeftRight() {
        [this.rightEdge, this.leftEdge] = [this.leftEdge, this.rightEdge];
        this.topEdge = reverseString(this.topEdge);
        this.bottomEdge = reverseString(this.bottomEdge);
        this.interior.forEach(row => row.reverse());
    }

    rotateClockwise() {
        [this.topEdge, this.rightEdge, this.bottomEdge, this.leftEdge] = [
            reverseString(this.leftEdge), this.topEdge,
            reverseString(this.rightEdge), this.bottomEdge];
        this.interior.reverse();
        this.interior = this.interior[0].map(
            (unused, index) => this.interior.map(row => row[index]));
    }

    clone() {
        return new Tile(this.allEdges().map(edge => edge.slice()),
                        this.id,
                        this.interior.map(row => row.slice()));
    }
}

function stringsToTile(rows, id) {
    const topEdge = rows[0];
    const rightEdge = rows.map(r => r[r.length - 1]).join('');
    const bottomEdge = rows[rows.length - 1];
    const leftEdge = rows.map(r => r[0]).join('');
    const interior = rows.slice(1, -1).map(r => r.split('').slice(1, -1));
    return new Tile([topEdge, rightEdge, bottomEdge, leftEdge], id, interior);
}

class Config {
    constructor(assembled, scrambled) {
        this.assembled = assembled;
        this.scrambled = scrambled;
    }
}

// Configs are sorted according to the number of already-assembled tiles
// to make solving faster.
class PrioritizedConfigs {
    constructor() {
        this.map = new Map();
        this.highestPriority = undefined;
    }

    push(config) {
        const priority = config.assembled.size;
        if (!this.map.has(priority)) {
            this.map.set(priority, []);
        }
        this.map.get(priority).push(config);
        if (this.highestPriority === undefined
            || this.highestPriority < priority) {
            this.highestPriority = priority;
        }
    }

    pop() {
        if (this.map.size === 0) {
            return null;
        }
        const config = this.map.get(this.highestPriority).shift();
        if (this.map.get(this.highestPriority).length === 0) {
            this.map.delete(this.highestPriority);
            this.highestPriority = Math.max(...this.map.keys());
        }
        return config;
    }

    empty() {
        return this.map.size === 0;
    }
}

function main20(input) {
    const tileStrings = input.split('\n\n');
    const scrambledTiles = [];
    for (const tileString of tileStrings) {
        const lines = splitIntoLines(tileString);
        if (lines.length === 0) {
            continue;
        }
        const match = lines[0].match(TILE_ID_REGEX);
        if (match === null) {
            console.log('No ID match for', tileString);
        }
        const id = Number(match.groups.id);
        if (Number.isNaN(id)) {
            console.log('Invalid ID for', tileString);
        }
        const newTile = stringsToTile(lines.slice(1), id);
        scrambledTiles.push(newTile);
    }

    const assembledMap = assemble(scrambledTiles);
    const minMaxXY = findCorners(assembledMap);
    const cornerProductPart1 = multiplyCornerIds(assembledMap, ...minMaxXY);
    const roughnessPart2 = calculateRoughness(assembledMap);

    return {'Part 1': cornerProductPart1.toString(), 'Part 2': roughnessPart2};
}

function assemble(scrambledTiles) {
    const assembedTiles = new Map();
    assembedTiles.set(coordToString(createCoord(0, 0)), scrambledTiles.pop());
    const configsToCheck = new PrioritizedConfigs;
    configsToCheck.push(new Config(assembedTiles, scrambledTiles));
    while (!configsToCheck.empty()) {
        const config = configsToCheck.pop();
        if (config.scrambled.length === 0) {
            return config.assembled;
        }
        for (let i = 0; i < config.scrambled.length; ++i) {
            const tileToInsert = config.scrambled.shift();
            const newAssemblies = insertTile(config.assembled, tileToInsert);
            if (newAssemblies.length > 0) {
                // This tile can fit in one or more places. Add all
                // potential assemblies to the list of ones to check.
                for (const assem of newAssemblies) {
                    configsToCheck.push(new Config(
                        assem, config.scrambled.slice()));
                }
                break;
            } else {
                // This tile doesn't fit anywhere right now, so
                // add it to the end of the scrambled tiles.
                config.scrambled.push(tileToInsert);
            }
        }
        // No new tiles can be inserted, so this assembly is a dead end
    }
    console.log('Failed to assemble tiles');
}

function insertTile(assembledMap, tileToInsert) {
    const newAssemblies = [];
    tileToInsert = tileToInsert.clone();
    const locationsTried = new Set();
    for (const [coordString, tile] of assembledMap) {
        const coord = coordFromString(coordString);
        const neighbors = getCardinalNeighbors(coord);
        // This order needs to match getCardinalNeighbors()
        const edges = ['topEdge', 'rightEdge',
                       'bottomEdge', 'leftEdge'];
        for (let e = 0; e < edges.length; ++e) {
            if (assembledMap.has(coordToString(neighbors[e]))) {
                // This edge already has a neighbor
                continue;
            }
            const insertionLocation = coordToString(neighbors[e]);
            if (locationsTried.has(insertionLocation)) {
                // We've already considered inserting this tile at this
                // location.
                continue;
            }
            locationsTried.add(insertionLocation);
            const edge = tile[edges[e]];
            const otherEdgeName = MATCHING_EDGES.get(edges[e]);
            for (let r = 0; r < 4; ++r) {
                for (let f = 0; f < 2; ++f) {
                    if (edge === tileToInsert[otherEdgeName]) {
                        // This tile fits here. Save it as
                        // a new potential assembly.
                        const newAssembly = new Map(assembledMap);
                        newAssembly.set(coordToString(neighbors[e]),
                                        tileToInsert.clone());
                        newAssemblies.push(newAssembly);
                    }
                    tileToInsert.flipLeftRight();
                }
                tileToInsert.rotateClockwise();
            }
        }
    }
    if (newAssemblies.length > 1) {
        console.log('Found multiple insertion spots for tile',
                    tileToInsert.id);
        return null;
    }
    return newAssemblies;
}

function getCardinalNeighbors(coord) {
    // Order is up, right, down, left
    const [x, y] = coord;
    return [createCoord(x, y - 1),
            createCoord(x + 1, y),
            createCoord(x, y + 1),
            createCoord(x - 1, y)];
}

function findCorners(assembledMap) {
    const allXCoords = Array.from(assembledMap.keys()).map(cs =>
        coordFromString(cs)[0]);
    const allYCoords = Array.from(assembledMap.keys()).map(cs =>
        coordFromString(cs)[1]);
    const minX = Math.min(...allXCoords);
    const maxX = Math.max(...allXCoords);
    const minY = Math.min(...allYCoords);
    const maxY = Math.max(...allYCoords);
    return [minX, maxX, minY, maxY];
}

function multiplyCornerIds(assembledMap, minX, maxX, minY, maxY) {
    let product = 1n;
    for (const x of [minX, maxX]) {
        for (const y of [minY, maxY]) {
            console.log('Corner at', [x, y]);
            const coordString = coordToString(createCoord(x, y));
            product *= BigInt(assembledMap.get(coordString).id);
        }
    }
    return product;
}

function calculateRoughness(assembledMap) {
    let assembledArray = mapToArray(assembledMap);
    if (assembledArray.length !== assembledArray[0].length) {
        console.log('Array is not square');
    }
    const imageWidth = assembledArray.length * assembledArray[0][0].interior.length;
    console.log('Image width', imageWidth);
    const templateWidth = SEA_MONSTER_TEMPLATE.split('\n')[0].length;
    console.log('Template width', templateWidth);
    const padding = imageWidth - templateWidth;
    console.log('Padding', padding);

    // Replace spaces in template with periods (wildcards).
    // Then join the lines into one, separating them with extra
    // periods so they are aligned properly.
    const monsterRegex = new RegExp(SEA_MONSTER_TEMPLATE.replaceAll(
        ' ', '.').split('\n').join('.'.repeat(padding)), 'g');
    console.log(monsterRegex);
    const pixelsPerMonster = [...SEA_MONSTER_TEMPLATE.matchAll(/#/g)].length;
    console.log('Each monster has', pixelsPerMonster, '# characters');

    let numMonsters = 0;
    let linearImage = arrayToLinearImage(assembledArray);
    for (let r = 0; r < 4; ++r) {
        for (let f = 0; f < 2; ++f) {
            linearImage = arrayToLinearImage(assembledArray);
            let monstersInOrientation = 0;
            // I tried using matchAll(), but that doesn't find overlapping
            // matches (monsters that share rows in the image), so I
            // iterate with search() instead.
            // Also, this also finds monsters that wrap around the edges
            // of the image, but apparently they don't show up in the input.
            let nextStart = linearImage.search(monsterRegex) + 1;
            while (nextStart >= 1) {
                ++monstersInOrientation;
                linearImage = linearImage.slice(nextStart);
                nextStart = linearImage.search(monsterRegex) + 1;
            }
            console.log('Found', monstersInOrientation,
                        'monsters in this orientation');
            numMonsters += monstersInOrientation;
            flipAssembly(assembledArray);
        }
        assembledArray = rotateAssembly(assembledArray);
    }

    console.log('Found', numMonsters, 'monsters in total');
    const monsterPixels = numMonsters * pixelsPerMonster;
    const roughness = [...linearImage.matchAll(/#/g)].length - monsterPixels;
    console.log('Roughness is', roughness);
    return roughness;
}

function mapToArray(assembledMap) {
    const [minX, maxX, minY, maxY] = findCorners(assembledMap);
    const assembledArray = [];
    for (let y = minY; y <= maxY; ++y) {
        const row = [];
        for (let x = minX; x <= maxX; ++x) {
            const tile = assembledMap.get(coordToString(createCoord(x, y)));
            row.push(tile);
        }
        assembledArray.push(row);
    }
    return assembledArray;
}

function flipAssembly(assembledArray) {
    assembledArray.reverse();
    assembledArray.forEach(row => row.forEach(tile => tile.flipUpDown()));
}

function rotateAssembly(assembledArray) {
    assembledArray.reverse();
    assembledArray = assembledArray[0].map(
        (unused, index) => assembledArray.map(row => row[index]));
    assembledArray.forEach(row => row.forEach(tile => tile.rotateClockwise()));
    return assembledArray;
}

function arrayToLinearImage(assembledArray) {
    const linearImage = assembledArray.map(
        row => row[0].interior.map(
            (unused, index) => row.map(
                tile => tile.interior[index].join('')
            ).join('')
        ).join('')
    ).join('');
    return linearImage;
}
