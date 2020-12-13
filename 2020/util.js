export {
    readFile, splitIntoLines, sumArray, setIntersection, setUnion,
    parseProgram, executeInstruction, runProgram, gcd, lcm
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
