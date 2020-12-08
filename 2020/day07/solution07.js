import {splitIntoLines, setUnion} from '../util.js';
export default main07;

const LINE_REGEX = /^(?<outerColor>[a-z ]+) bags contain(?:(?<contents>( ([0-9]+) ([a-z ]+) bags?[,.])+)|(?<empty> no other bags.))$/;
const CONTENTS_REGEX = /(?<number>[0-9]+) (?<color>[a-z ]+) bags?/g;
const SELECTED_COLOR = 'shiny gold';

function main07(input) {
    const lines = splitIntoLines(input);
    const allBags = {};

    for (let line of lines) {
        const match = line.match(LINE_REGEX);
        if (match !== null) {
            const children = [];
            if (match.groups.contents !== undefined) {
                const contentsMatch = match.groups.contents.matchAll(
                    CONTENTS_REGEX);
                for (let subMatch of contentsMatch) {
                    children.push({
                        color: subMatch.groups.color,
                        number: subMatch.groups.number
                    });
                }
            }
            allBags[match.groups.outerColor] = children;
            console.log(match.groups.outerColor, children);
        }
    }

    const colorsPartA = findContainingColors(SELECTED_COLOR, allBags, {});
    const countPartB = countContainedBags(SELECTED_COLOR, allBags, {});

    return {'Part A': colorsPartA.size, 'Part B': countPartB};
}

function findContainingColors(color, allBags, memo) {
    if (color in memo) {
        console.log('Memo', color, memo[color]);
        return memo[color];
    }

    const directContainingColors = new Set();
    for (const parentColor in allBags) {
        for (const parentChild of allBags[parentColor]) {
            if (parentChild.color == color) {
                directContainingColors.add(parentColor);
                break;
            }
        }
    }

    let allContainingColors = new Set(directContainingColors);
    for (const parentColor of directContainingColors) {
        allContainingColors = setUnion(
            allContainingColors,
            findContainingColors(parentColor, allBags, memo));
    }

    memo[color] = allContainingColors;
    console.log(color, 'contained by', allContainingColors);
    return allContainingColors;
}

function countContainedBags(color, allBags, memo) {
    if (color in memo) {
        console.log('Memo', color, memo[color]);
        return memo[color];
    }

    let totalContainedBags = 0;
    for (const childNode of allBags[color]) {
        totalContainedBags += childNode.number * (1 + countContainedBags(
            childNode.color, allBags, memo));
    }

    memo[color] = totalContainedBags;
    console.log(color, 'contains', totalContainedBags, 'bags');
    return totalContainedBags;
}
