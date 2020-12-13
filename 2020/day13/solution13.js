import {splitIntoLines, lcm} from '../util.js';
export default main13;

function main13(input) {
    const [timestamp, busIDsStr] = splitIntoLines(input);
    const busIDs = busIDsStr.split(',').map(s => Number(s));

    let productPart1 = undefined;
    let shortestWaitTime = Number.POSITIVE_INFINITY;
    for (const id of busIDs.filter(n => !Number.isNaN(n))) {
        const waitTime = id - (timestamp % id);
        if (waitTime < shortestWaitTime) {
            console.log('Shortest wait so far:', waitTime);
            shortestWaitTime = waitTime;
            productPart1 = waitTime * id;
        }
    }

    const nonNanIDs = busIDs.filter(n => !Number.isNaN(n));
    const sortedIDs = nonNanIDs.sort((a, b) => b - a);
    const indices = sortedIDs.map(n => BigInt(busIDs.indexOf(n)));
    console.log(indices);
    const bigintIDs = sortedIDs.map(n => BigInt(n));
    console.log(bigintIDs);
    let timestampPart2 = undefined;
    let increment = bigintIDs[0];
    let nextIndex = 1;
    for (let t = -indices[0]; nextIndex < bigintIDs.length; t += increment) {
        if ((t + indices[nextIndex]) % bigintIDs[nextIndex] === 0n) {
            increment = lcm(increment, bigintIDs[nextIndex]);
            console.log('New increment', increment);
            ++nextIndex;
            timestampPart2 = t;
        }
    }

    return {'Part 1': productPart1, 'Part 2': timestampPart2.toString()};
}
