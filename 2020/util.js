export {
    readFile, splitIntoLines, sumArray, setIntersection, setUnion,
    parseProgram, executeInstruction, runProgram, gcd, lcm, toBinary,
    createCoord, coordToString, coordFromString, getNeighboringCoords
};

function readFile(fileName, callback) {
    console.log('Reading from ', fileName);
    const request = new XMLHttpRequest();
    request.open('GET', fileName);
    request.onload = function () {
        if (request.status === 200) {
            callback(request.responseText);
        } else {
            console.log('Failed to load file ' + fileName);
            console.log(request.status);
            console.log(request.statusText);
        }
    };

    request.send();
}

function splitIntoLines(fileContents, stripEmptyLines=true) {
    let lines = fileContents.split('\n');
    if (stripEmptyLines) {
        lines = lines.filter(line => line.length > 0);
    }
    return lines;
}

function sumArray(array) {
    return array.reduce(
        (runningSum, currentValue) => runningSum + currentValue);
}

function setIntersection(setA, setB) {
    const intersection = new Set();
    for (let item of setA) {
        if (setB.has(item)) {
            intersection.add(item);
        }
    }
    return intersection;
}

function setUnion(setA, setB) {
    const union = new Set(setA);
    for (let item of setB) {
        union.add(item);
    }
    return union;
}

function parseProgram(lines) {
    const ACC_REGEX = /acc (?<argument>[+-][0-9]+)/;
    const JMP_REGEX = /jmp (?<argument>[+-][0-9]+)/;
    const NOP_REGEX = /nop (?<argument>[+-][0-9]+)/;

    const instructions = [];

    for (const line of lines) {
        let match = line.match(ACC_REGEX);
        if (match !== null) {
            instructions.push({
                operator: 'acc',
                argument: Number(match.groups.argument)
            });
            continue;
        }
        match = line.match(JMP_REGEX);
        if (match !== null) {
            instructions.push({
                operator: 'jmp',
                argument: Number(match.groups.argument)
            });
            continue;
        }
        match = line.match(NOP_REGEX);
        if (match !== null) {
            instructions.push({
                operator: 'nop',
                argument: Number(match.groups.argument)
            });
            continue;
        }
        console.log('Unable to parse line', line);
    }

    return instructions;
}

function executeInstruction(state, instructions) {
    const newState = {...state};
    const instruction = instructions[state.programCounter];
    // console.log('Instruction', instruction);
    switch (instruction.operator) {
        case 'acc':
            newState.accumulator += instruction.argument;
            ++newState.programCounter;
            break;
        case 'jmp':
            newState.programCounter += instruction.argument;
            break;
        case 'nop':
            ++newState.programCounter;
            break;
        default:
            console.log('Unknown instruction', instruction);
    }
    // console.log('State change', state, '->', newState);
    return newState;
}

function runProgram(instructions, state) {
    if (state === undefined) {
        state = {
            programCounter: 0,
            accumulator: 0
        };
    }
    const seenInstructions = new Set();
    while (!seenInstructions.has(state.programCounter)
           && state.programCounter !== instructions.length) {
        seenInstructions.add(state.programCounter);
        state = executeInstruction(state, instructions);
    }
    const terminates = state.programCounter === instructions.length;
    return {state: state, terminates: terminates};
}

function gcd(a, b) {
    // Greatest common divisor using Euclid's algorithm
    // https://en.wikipedia.org/wiki/Euclidean_algorithm
    while (b != 0) {
        const temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

function lcm(a, b) {
    // https://en.wikipedia.org/wiki/Least_common_multiple
    return (a / gcd(a, b)) * b;
}

function toBinary(number, width) {
    let binary = number.toString(2);
    while (width !== undefined && binary.length < width) {
        binary = '0'.concat(binary);
    }
    return binary;
}

function createCoord(x, y, z, w) {
    if (z === undefined) {
        return [x, y];
    }
    if (w === undefined) {
        return [x, y, z];
    }
    return [x, y, z, w];
}

// Coordinates are sometimes represented as strings
// so they can be used in Sets and Maps.

function coordToString(coord) {
    return coord.toString();
}

function coordFromString(str) {
    return str.split(',').map(s => Number(s));
}

function getNeighboringCoords(coord) {
    switch (coord.length) {
        case 2:
            return getNeighboringCoords2d(coord);
        case 3:
            return getNeighboringCoords3d(coord);
        case 4:
            return getNeighboringCoords4d(coord);
        default:
            throw new Error('Invalid coordinate length');
    }
}

function getNeighboringCoords2d(coord) {
    const neighbors = [];
    const [x, y] = coord;
    for (let i = x - 1; i <= x + 1; ++i) {
        for (let j = y - 1; j <= y + 1; ++j) {
            if (i === x && j === y) {
                continue;
            }
            neighbors.push(createCoord(i, j));
        }
    }
    if (neighbors.length != 8) {
        throw new Error('Number of 2D neighbors is not 8');
    }
    return neighbors;
}

function getNeighboringCoords3d(coord) {
    const neighbors3d = [];
    const [x, y, z] = coord;
    const neighbors2d = getNeighboringCoords2d(coord.slice(0, 2));
    for (let k = z - 1; k <= z + 1; ++k) {
        for (const neigh2d of neighbors2d) {
            neighbors3d.push(neigh2d.concat([k]));
        }
    }
    // neighbors2d doesn't contain [x, y], so we need to add the
    // two points in front of and behind that explicitly.
    neighbors3d.push(createCoord(x, y, z - 1));
    neighbors3d.push(createCoord(x, y, z + 1));
    if (neighbors3d.length != 26) {
        throw new Error('Number of 3D neighbors is not 26');
    }
    return neighbors3d;
}

function getNeighboringCoords4d(coord) {
    const neighbors4d = [];
    const [x, y, z, w] = coord;
    const neighbords3d = getNeighboringCoords3d(coord.slice(0, 3));
    for (let l = w - 1; l <= w + 1; ++l) {
        for (const neigh3d of neighbords3d) {
            neighbors4d.push(neigh3d.concat([l]));
        }
    }
    // neighbors3d doesn't contain [x, y, z], so we need to add the
    // two points "above" and "below" that explicity.
    neighbors4d.push(createCoord(x, y, z, w - 1));
    neighbors4d.push(createCoord(x, y, z, w + 1));
    if (neighbors4d.length != 80) {
        throw new Error('Number of 4D neighbors is not 80');
    }
    return neighbors4d;
}
