export default main15;

const PART1_ORDINAL = 2020;
const PART2_ORDINAL = 30000000;

function main15(input) {
    const numberArray = input.split(',').map(s => Number(s));
    // numberMap maps values to their most recent index
    const numberMap = new Map();
    numberArray.slice(0, -1).forEach((n, index) => {
        numberMap.set(n, index);
    });
    let mostRecentNumber = numberArray[numberArray.length - 1];
    let iteration = numberArray.length;

    let answerPart1 = undefined;
    let answerPart2 = undefined;

    while (iteration <= Math.max(PART1_ORDINAL, PART2_ORDINAL)) {
        const next = getNextNumber(numberMap, mostRecentNumber, iteration);
        numberMap.set(mostRecentNumber, iteration - 1);
        if (iteration === PART1_ORDINAL - 1) {
            answerPart1 = next;
        } else if (iteration === PART2_ORDINAL - 1) {
            answerPart2 = next;
        }
        mostRecentNumber = next;
        ++iteration;
    }

    return {'Part 1': answerPart1, 'Part 2': answerPart2};
}

function getNextNumber(numberMap, mostRecentNumber, iteration) {
    if (numberMap.has(mostRecentNumber)) {
        return iteration - numberMap.get(mostRecentNumber) - 1;
    }
    return 0;
}
