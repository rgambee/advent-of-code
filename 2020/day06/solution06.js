import {splitIntoLines, setIntersection} from '../util.js';
export default main06;

function main06(input) {
    const lines = splitIntoLines(input, false);

    let anyYes = new Set();
    let allYes = null;
    let anyCountA = 0;
    let allCountB = 0;

    for (let line of lines) {
        if (line.length == 0) {
            console.log('Counting answers', anyYes, allYes);
            anyCountA += anyYes.size;
            allCountB += allYes.size;
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

    return {'Part A': anyCountA, 'Part B': allCountB};
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
