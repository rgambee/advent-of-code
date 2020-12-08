import {splitIntoLines} from '../util.js';
export default main05;

const LINE_REGEX = /^(?<row>[FB]{7})(?<col>[LR]{3})$/;

function main05(input) {
    const lines = splitIntoLines(input);

    let highestIdPart1 = -1;
    const allIds = [];
    for (let line of lines) {
        const decoded = decodeLine(line);
        if (!decoded) {
            continue;
        }
        const decodedId = decodedToId(decoded);
        if (decodedId > highestIdPart1) {
            highestIdPart1 = decodedId;
        }
        allIds.push(decodedId);
    }

    let missingIdPart2 = searchForMissingId(allIds);

    return {'Part 1': highestIdPart1, 'Part 2': missingIdPart2};
}

function decodeLine(line) {
    const match = line.match(LINE_REGEX);
    if (match === null || match.length == 0) {
        console.log('Could not decode ', line);
        return null;
    }
    let rowStr = match.groups.row;
    rowStr = rowStr.replaceAll('F', '0').replaceAll('B', '1');
    let colStr = match.groups.col;
    colStr = colStr.replaceAll('L', '0').replaceAll('R', '1');
    const decoded = {row: Number.parseInt(rowStr, 2),
                     col: Number.parseInt(colStr, 2)};
    console.log('Decoded ', line, ' to ', decoded);
    return decoded;
}

function decodedToId(decoded) {
    const id = decoded.row * 8 + decoded.col;
    console.log('ID is ', id);
    return id;
}

function searchForMissingId(allIds) {
    allIds.sort();
    for (let i = 1; i < allIds.length; ++i) {
        if (allIds[i-1] == allIds[i] - 2) {
            console.log('Found gap of 1: ', allIds.slice(i - 1, i + 1));
            return allIds[i] - 1;
        }
    }
    console.log('Failed to find missing id');
    return -1;
}
