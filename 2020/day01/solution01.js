import {splitIntoLines} from '../util.js';
export default main01;

const DESIRED_SUM = 2020;

function main01(input) {
    const lines = splitIntoLines(input);
    const numbers = [];
    lines.forEach(line => numbers.push(Number(line)));
    const selectedPart1 = findPermutation(numbers, 2);
    console.log('Found values for part 1: ', selectedPart1);
    const productPart1 = multiply(selectedPart1);
    const selectedPart2 = findPermutation(numbers, 3);
    console.log('Found values for part 2: ', selectedPart2);
    const productPart2 = multiply(selectedPart2);
    return {'Part 1': productPart1, 'Part 2': productPart2};
}

function multiply(numbers) {
    let product = 1;
    for (let num of numbers) {
        product *= num;
    }
    return product;
}

function findPermutation(numbers, permulationLength) {
    const indices = [];
    for (let n = permulationLength - 1; n >= 0; --n) {
        indices.push(n);
    }

    while (!done(indices, numbers.length)) {
        let sum = 0;
        for (let ind of indices) {
            sum += numbers[ind];
        }
        if (sum === DESIRED_SUM) {
            console.log('Found indices: ', indices);
            const selectedValues = [];
            indices.forEach(ind => selectedValues.push(numbers[ind]));
            return selectedValues;
        }
        increment(indices, numbers.length);
    }

    console.log('No set of numbers found which add to the desired total');
    return [];
}

function done(indices, length) {
    for (let ind of indices) {
        if (ind < length) {
            return false;
        }
    }
    return true;
}

function increment(indices, length) {
    for (let i = 0; i < indices.length; ++i) {
        ++indices[i];
        if (indices[i] >= length) {
            indices[i] = 0;
        } else {
            return;
        }
    }
    console.log('Ran out of indices to increment');
}
