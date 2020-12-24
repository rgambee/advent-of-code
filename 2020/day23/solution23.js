export default main23;

const NUM_MOVES_PART1 = 100;
const NUM_MOVES_PART2 = 10000000;
const NUM_CUPS_PART2 = 1000000;

class Cup {
    constructor(value, next) {
        this.value = value;
        this.next = next;
    }
}

function main23(input) {
    const startingCupNumbers = input.trim().split('').map(s => Number(s));
    let [currentCup, cupArrayPart1] = assembleCups(startingCupNumbers);
    for (let i = 0; i < NUM_MOVES_PART1; ++i) {
        currentCup = singleMove(currentCup, cupArrayPart1);
    }
    const orderPart1 = getFinalOrder(cupArrayPart1);

    const cupAssemblyPart2 = assembleCups(startingCupNumbers, NUM_CUPS_PART2);
    currentCup = cupAssemblyPart2[0];
    const cupArrayPart2 = cupAssemblyPart2[1];
    for (let i = 0; i < NUM_MOVES_PART2; ++i) {
        currentCup = singleMove(currentCup, cupArrayPart2);
    }
    const cup1 = cupArrayPart2[1];
    console.log('Cup numbers following 1 are',
                cup1.next.value, cup1.next.next.value);
    const productPart2 = (BigInt(cupArrayPart2[1].next.value)
                          * BigInt(cupArrayPart2[1].next.next.value));

    return {'Part 1': orderPart1, 'Part 2': productPart2.toString()};
}

function assembleCups(startingCupNumbers, totalNumberOfCups) {
    if (totalNumberOfCups === undefined) {
        totalNumberOfCups = startingCupNumbers.length;
    }
    const cupNumbers = startingCupNumbers.slice();
    let nextCupNumber = Math.max(...startingCupNumbers);
    while (cupNumbers.length < totalNumberOfCups) {
        cupNumbers.push(++nextCupNumber);
    }

    let firstCup = null;
    let previousCup = null;
    const cupArray = Array(totalNumberOfCups + 1);
    for (const num of cupNumbers) {
        const newCup = new Cup(num, null, null);
        cupArray[num] = newCup;
        if (firstCup === null) {
            firstCup = newCup;
        }
        if (previousCup !== null) {
            previousCup.next = newCup;
        }
        previousCup = newCup;
    }
    // Make the linked list circular
    previousCup.next = firstCup;
    return [firstCup, cupArray];
}

function singleMove(currentCup, cupArray) {
    const firstCupToMove = currentCup.next;
    const numbersToMove = [firstCupToMove.value,
                           firstCupToMove.next.value,
                           firstCupToMove.next.next.value];
    let cupNumberToInsertAfter = currentCup.value > 1 ?
        currentCup.value - 1 : cupArray.length - 1;
    while (numbersToMove.includes(cupNumberToInsertAfter)) {
        cupNumberToInsertAfter = cupNumberToInsertAfter > 1 ?
            cupNumberToInsertAfter - 1 : cupArray.length - 1;
    }
    currentCup.next = firstCupToMove.next.next.next;
    const destCup = cupArray[cupNumberToInsertAfter];
    firstCupToMove.next.next.next = destCup.next;
    destCup.next = firstCupToMove;
    return currentCup.next;
}

function getFinalOrder(cupArray) {
    const cup1 = cupArray[1];
    const cupNumbers = [];
    let currentCup = cup1.next;
    while (currentCup !== cup1) {
        cupNumbers.push(currentCup.value);
        currentCup = currentCup.next;
    }
    return cupNumbers.join('');
}
