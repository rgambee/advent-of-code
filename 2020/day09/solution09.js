import {splitIntoLines, sumArray} from '../util.js';
export default main09;

const PREAMBLE = 25;
const LOOKBACK = PREAMBLE;

function main09(input) {
    const lines = splitIntoLines(input);
    const numbers = lines.map(line => Number(line));
    let firstInvalidPart1 = undefined;
    for (let index = 0; index < numbers.length; ++index) {
        if (!isValidAt(numbers, index)) {
            firstInvalidPart1 = numbers[index];
            break;
        }
    }

    const matchingSequence = findSequence(numbers, firstInvalidPart1);
    const minValue = Math.min(...matchingSequence);
    const maxValue = Math.max(...matchingSequence);

    return {'Part 1': firstInvalidPart1, 'Part 2': minValue + maxValue};
}

function isValidAt(numbers, index) {
    if (index < PREAMBLE) {
        return true;
    }
    for (let i = index - LOOKBACK; i < index; ++i) {
        for (let j = i + 1; j < index; ++j) {
            if (numbers[i] === numbers[j]) {
                // We know i != j, so this could only
                // happen if numbers are repeated.
                continue;
            }
            if (numbers[i] + numbers[j] === numbers[index]) {
                console.log('[', index, ']', numbers[index], '==',
                            '[', i, ']', numbers[i], '+',
                            '[', j, ']', numbers[j]);
                return true;
            }
        }
    }
    console.log('Invalid: [', index, ']', numbers[index]);
    return false;
}

function findSequence(numbers, desiredSum) {
    let start = 0;
    let end = 1;
    let sliceSum = sumArray(numbers.slice(start, end));
    while (sliceSum !== desiredSum) {
        if (sliceSum < desiredSum) {
            ++end;
        } else {
            ++start;
        }
        sliceSum = sumArray(numbers.slice(start, end));
    }
    return numbers.slice(start, end);
}
