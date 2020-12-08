import {splitIntoLines, parseProgram, runProgram} from '../util.js';
export default main08;

function main08(input) {
    const lines = splitIntoLines(input);
    const instructions = parseProgram(lines);

    let programResult = runProgram(instructions);
    const accumulatorPartA = programResult.state.accumulator;

    let startingIndex = 0;
    while (!programResult.terminates) {
        const updated = swapInstruction(instructions, startingIndex);
        startingIndex = updated.startingIndex;
        programResult = runProgram(updated.program);
    }

    return {'Part 1': accumulatorPartA,
            'Part 2': programResult.state.accumulator};
}

function swapInstruction(originalProgram, startingIndex) {
    const newProgram = [...originalProgram];    // Shallow copy only!
    for (let i = startingIndex; i < newProgram.length; ++i) {
        const instr = newProgram[i];
        const newInstr = {...instr};
        if (instr.operator !== 'jmp' && instr.operator !== 'nop') {
            continue;
        }
        if (instr.operator === 'jmp') {
            newInstr.operator = 'nop';
        }
        else if (instr.operator === 'nop') {
            newInstr.operator = 'jmp';
        }
        console.log('Changing instruction at index', i, instr, '->', newInstr);
        newProgram[i] = newInstr;
        startingIndex = i + 1;
        break;
    }
    return {program: newProgram, startingIndex: startingIndex};
}
