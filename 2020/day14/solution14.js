import {splitIntoLines, toBinary} from '../util.js';
export default main14;

const WORD_LENGTH = 36;
const MASK_REGEX = /^mask = (?<mask>[01X]{36})$/;
const MEM_REGEX = /^mem\[(?<address>[0-9]+)\] = (?<value>[0-9]+)$/;

function main14(input) {
    const instructions = parseInstructions(splitIntoLines(input));
    let state = {
        // memory is a map of strings (representing addresses in decimal)
        // to decimal numbers.
        memory: {},
        mask: 'X'.repeat(WORD_LENGTH)
    };
    runProgram(instructions, state, 1);
    let sumPart1 = 0n;
    for (const address in state.memory) {
        sumPart1 += BigInt(state.memory[address]);
    }

    state = {
        memory: {},
        mask: 'X'.repeat(WORD_LENGTH)
    };
    runProgram(instructions, state, 2);
    let sumPart2 = 0n;
    for (const address in state.memory) {
        sumPart2 += BigInt(state.memory[address]);
    }

    return {'Part 1': sumPart1.toString(), 'Part 2': sumPart2.toString()};
}

function parseInstructions(lines) {
    const instructions = [];
    for (const line of lines) {
        const maskMatch = line.match(MASK_REGEX);
        if (maskMatch !==  null) {
            instructions.push({
                operation: 'mask',
                value: maskMatch.groups.mask        // Leave as string
            });
            continue;
        }
        const memMatch = line.match(MEM_REGEX);
        if (memMatch !== null) {
            instructions.push({
                operation: 'mem',
                address: memMatch.groups.address,  // Also leave as string
                value: Number(memMatch.groups.value)
            });
            continue;
        }
        console.log('Cannot parse instruction', line);
    }
    return instructions;
}

function runProgram(instructions, state, version) {
    for (const instruction of instructions) {
        executeInstruction(instruction, state, version);
    }
}

function executeInstruction(instruction, state, version) {
    if (instruction.operation === 'mask') {
        state.mask = instruction.value;
    } else if (instruction.operation === 'mem') {
        if (version === 1) {
            state.memory[instruction.address] = Number.parseInt(applyMask(
                state.mask, instruction.value, version), 2);
        } else if (version === 2) {
            const maskedAddress = applyMask(
                state.mask, Number(instruction.address), version);
            for (const address of enumerateAddresses(maskedAddress)) {
                // Convert address from a string representing a binary
                // number to a string representing a decimal number.
                const addressDec = Number.parseInt(address, 2).toString();
                state.memory[addressDec] = instruction.value;
            }
        }
    } else {
        console.log('Invalid instruction', instruction);
    }
}

function applyMask(mask, value, version) {
    let binaryString = toBinary(value, WORD_LENGTH);
    const stringArray = binaryString.split('');
    for (let i = 0; i < WORD_LENGTH; ++i) {
        if (version === 1) {
            if (mask[i] === '0') {
                stringArray[i] = '0';
            } else if (mask[i] === '1') {
                stringArray[i] = '1';
            }
        } else if (version === 2) {
            if (mask[i] === '1') {
                stringArray[i] = '1';
            } else if (mask[i] === 'X') {
                stringArray[i] = 'X';
            }
        }
    }
    return ''.concat(...stringArray);
}

function enumerateAddresses(mask) {
    if (!mask.includes('X')) {
        return [mask];
    }
    return enumerateAddresses(mask.replace('X', '0')).concat(
        enumerateAddresses(mask.replace('X', '1')));
}
