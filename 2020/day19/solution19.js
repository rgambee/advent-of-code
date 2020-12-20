import {splitIntoLines} from '../util.js';
export default main19;

const LITERAL_MATCH = /^(?<ruleNumber>[0-9]+): "(?<letter>[ab])"$/;
const INDIRECT_MATCH = /^(?<ruleNumber>[0-9]+): (?<subrules>[0-9 |]+)$/;
// The updated rule 8 matches 1 or more rule 42s.
// The updated rule 11 matches 1 or more 42s followed by an equal number of 31s.
const PART2_RULE_CHANGES = `8: 42 | 42 8
11: 42 31 | 42 11 31`;

class Rule {
    constructor(subrules) {
        // subrules is an array of arrays. This rule matches a message
        // if ANY of the elements of subrules matches the message. Those
        // inner arrays match if ALL of their elements match. For example,
        //      subrules = [[1, 2], [3, 4]]
        // is equivalent to
        //      1 2 | 3 4
        this.subrules = subrules;
    }

    matches(message, rules) {
        // TODO use memoization to improve speed
        const remainingAfterMatches = [];
        for (const subrule of this.subrules) {
            let remainingMessages = [message.slice(0)];
            for (const subsubrule of subrule) {
                const nextRemainingMessages = [];
                for (const remaining of remainingMessages) {
                    if (Number.isInteger(subsubrule)) {
                        // Subsubrule refers to another rule, so check whether
                        // it matches the remaining message.
                        const subsubmatches = rules.get(subsubrule).matches(
                            remaining, rules);
                        nextRemainingMessages.push(...subsubmatches);
                    } else {
                        // This subsubrule is a literal string like "a".
                        if (remaining.length > 0 && remaining[0] === subsubrule) {
                            // It matches, so remove the first letter of the
                            // message and save the remaining part for later
                            // consideration.
                            nextRemainingMessages.push(remaining.slice(1));
                        }
                    }
                }
                remainingMessages = nextRemainingMessages;
            }
            // This subrule has consumed everything it can. Save the portions
            // it didn't necessarily consume and continue to the next subrule.
            remainingAfterMatches.push(...remainingMessages);
        }
        return remainingAfterMatches;
    }
}

function main19(input) {
    const stripEmptyLines = false;
    const lines = splitIntoLines(input, stripEmptyLines);
    const rules = new Map();
    while (lines.length > 0) {
        const line = lines.shift();
        if (line === '') {
            // Done reading rules
            break;
        }
        parseRule(rules, line);
    }
    console.log('Parsed', rules.size, 'rules');

    const matchesPart1 = countMatches(rules, lines);
    console.log('Part 1:', matchesPart1, 'matches out of', lines.length,
                'messages');
    // Modify rules for part 2
    for (const line of splitIntoLines(PART2_RULE_CHANGES)) {
        parseRule(rules, line);
    }
    console.log(8, rules.get(8));
    console.log(11, rules.get(11));
    const matchesPart2 = countMatches(rules, lines);
    console.log('Part 2:', matchesPart2, 'matches out of', lines.length,
                'messages');

    return {'Part 1': matchesPart1, 'Part 2': matchesPart2};
}

function parseRule(rules, line) {
    let match = line.match(LITERAL_MATCH);
    if (match !== null) {
        const ruleNumber = Number(match.groups.ruleNumber);
        rules.set(ruleNumber, new Rule([[match.groups.letter]]));
        return;
    }
    match = line.match(INDIRECT_MATCH);
    if (match !== null) {
        const ruleNumber = Number(match.groups.ruleNumber);
        const subrules = [[]];
        let subruleIndex = 0;
        for (const char of match.groups.subrules.split(' ')) {
            if (char === '|') {
                // Start a new subrule
                subrules.push([]);
                ++subruleIndex;
            } else {
                const subsubrule = Number(char);
                if (Number.isNaN(subsubrule)) {
                    console.log('Invalid rule', char, 'in line', line);
                }
                subrules[subruleIndex].push(subsubrule);
            }
        }
        rules.set(ruleNumber, new Rule(subrules));
        return;
    }
    console.log('Invalid rule', line);
}

function countMatches(rules, messages) {
    let matchingMessages = 0;
    for (const message of messages) {
        if (messages.length === 0) {
            continue;
        }
        const leftovers = rules.get(0).matches(message, rules);
        if (leftovers.some(l => l.length === 0)) {
            ++matchingMessages;
        }
    }
    return matchingMessages;
}
