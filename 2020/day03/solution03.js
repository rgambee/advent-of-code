import {splitIntoLines} from '../util.js';
export default main03;

const DOWNS_TO_TRY = [1, 1, 1, 1, 2];
const RIGHTS_TO_TRY = [1, 3, 5, 7, 1];
const PART_1_DOWN = 1;
const PART_1_RIGHT = 3;

function main03(input) {
    const treeMap = splitIntoLines(input);

    let treeCountPart1;
    let treeProductPart2 = 1;
    DOWNS_TO_TRY.forEach(function(down, index) {
        const right = RIGHTS_TO_TRY[index];
        let treeCount = countTreesForSlope(treeMap, right, down);
        treeProductPart2 *= treeCount;
        if (down === PART_1_DOWN && right === PART_1_RIGHT) {
            treeCountPart1 = treeCount;
        }
    });

    return {'Part 1': treeCountPart1, 'Part 2': treeProductPart2};
}

function countTreesForSlope(treeMap, right, down) {
    let row = 0;
    let col = 0;
    let treeCount = 0;
    while (row < treeMap.length) {
        if (isTreeAt(treeMap, row, col)) {
            ++treeCount;
        }
        row += down;
        col += right;
    }
    return treeCount;
}

function isTreeAt(treeMap, row, col) {
    return treeMap[row][col % treeMap[row].length] === '#';
}
