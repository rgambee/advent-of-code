import {readFile, splitIntoLines} from '../util.js';
export {main03};

const DOWNS_TO_TRY = [1, 1, 1, 1, 2];
const RIGHTS_TO_TRY = [1, 3, 5, 7, 1];
const PART_A_DOWN = 1;
const PART_A_RIGHT = 3;

function main03(filePath) {
    const treeMap = splitIntoLines(readFile(filePath));

    let treeCountA;
    let treeProductB = 1;
    DOWNS_TO_TRY.forEach(function(down, index) {
        const right = RIGHTS_TO_TRY[index];
        let treeCount = countTreesForSlope(treeMap, right, down);
        treeProductB *= treeCount;
        if (down === PART_A_DOWN && right === PART_A_RIGHT) {
            treeCountA = treeCount;
        }
    });

    return {'Part A': treeCountA, 'Part B': treeProductB    };
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
