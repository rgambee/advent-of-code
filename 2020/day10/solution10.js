import {splitIntoLines} from '../util.js';
export default main10;


function main10(input) {
    const lines = splitIntoLines(input);
    const numbers = lines.map(line => Number(line));
    // Insert 0 to represent outlet
    numbers.push(0);
    numbers.sort((a, b) => a - b);
    // Insert value to represent device's internal adapter
    numbers.push(numbers[numbers.length - 1] + 3);

    let diffBy1 = 0;
    let diffBy3 = 0;
    let runningDiffBy1 = 0;
    let arrangements = 1;
    const memo = {};
    // Numbers that have a gap of 3 on either side can't be removed
    // (otherwise they'd create a gap of at least 4). Numbers with
    // gaps of 1 on both sides can be removed, but we need to be
    // sure not to remove three in a row.
    for (let i = 1; i < numbers.length; ++i) {
        if (numbers[i] - numbers[i-1] === 1) {
            ++diffBy1;
            ++runningDiffBy1;
        }
        else if (numbers[i] - numbers[i-1] === 3) {
            ++diffBy3;
            arrangements *= countArrangements(runningDiffBy1, memo);
            runningDiffBy1 = 0;
        } else {
            console.log('Difference is not 1 or 3', numbers[i-1], numbers[i]);
        }
    }

    console.log('Differences of 1', diffBy1);
    console.log('Differences of 3', diffBy3);

    return {'Part 1': diffBy1 * diffBy3, 'Part 2': arrangements};
}

function countArrangements(consecutiveDiffBy1, memo) {
    if (consecutiveDiffBy1 in memo) {
        return memo[consecutiveDiffBy1];
    }

    console.log('Counting arrangements for', consecutiveDiffBy1);
    let count = 0;
    for (let i = 0; i < 2 ** (consecutiveDiffBy1 - 1); ++i) {
        const binary = toBinary(i, consecutiveDiffBy1 - 1);
        if (!binary.includes('000')) {
            ++count;
        }
    }
    console.log('Result: ', count);
    memo[consecutiveDiffBy1] = count;
    return count;
}

function toBinary(number, width) {
    let binary = number.toString(2);
    while (width !== undefined && binary.length < width) {
        binary = '0'.concat(binary);
    }
    return binary;
}