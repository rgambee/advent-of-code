import {splitIntoLines} from '../util.js';
export default main02;

const LINE_REGEX = /^(?<min>[0-9]+)-(?<max>[0-9]+) (?<letter>[a-z]): (?<password>[a-z]+)$/;

function main02(input) {
    const lines = splitIntoLines(input);
    let validCountPart1 = 0;
    let validCountPart2 = 0;
    for (let line of lines) {
        const match = line.match(LINE_REGEX);
        if (!match) {
            continue;
        }
        if (isValidPart1(match.groups.min, match.groups.max,
                         match.groups.letter, match.groups.password)) {
            ++validCountPart1;
        }
        if (isValidPart2(match.groups.min, match.groups.max,
                         match.groups.letter, match.groups.password)) {
            ++validCountPart2;
        }
    }
    return {'Part 1': validCountPart1, 'Part 2': validCountPart2};
}

function isValidPart1(minCount, maxCount, letter, password) {
    let letterCount = 0;
    const letterMatch = password.match(new RegExp(letter, 'g'));
    if (letterMatch) {
        letterCount = letterMatch.length;
    }
    return (minCount <= letterCount && letterCount <= maxCount);
}

function isValidPart2(firstIndex1, secondIndex1, letter, password) {
    const firstIndex0 = firstIndex1 - 1;
    const secondIndex0 = secondIndex1 - 1;
    if (firstIndex0 < 0 || firstIndex0 >= password.length
        || secondIndex0 < 0 || secondIndex0 >= password.length) {
        return false;
    }
    const firstMatch = password[firstIndex0] === letter;
    const secondMatch = password[secondIndex0] === letter;
    return ((firstMatch || secondMatch) && !(firstMatch && secondMatch));
}
