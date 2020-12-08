import {splitIntoLines, setIntersection} from '../util.js';
export default main06;

function main06(input) {
    const lines = splitIntoLines(input, false);

    let anyYes = new Set();
    let allYes = null;
    let anyCountPart1 = 0;
    let allCountPart2 = 0;

    for (let line of lines) {
        if (line.length == 0) {
            console.log('Counting answers', anyYes, allYes);
            anyCountPart1 += anyYes.size;
            allCountPart2 += allYes.size;
            anyYes.clear();
            allYes = null;
        } else {
            console.log('Adding answers', line);
            anyYes = stringToSet(line, anyYes);
            if (allYes === null) {
                allYes = stringToSet(line);
            } else {
                allYes = setIntersection(allYes, stringToSet(line));
            }
        }
    }

    return {'Part 1': anyCountPart1, 'Part 2': allCountPart2};
}

function stringToSet(inputString, startingSet) {
    let set = new Set();
    if (startingSet !== undefined) {
        set = new Set(startingSet);
    }
    for (let letter of inputString) {
        set.add(letter);
    }
    return set;
}
