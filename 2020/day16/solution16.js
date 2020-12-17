import {splitIntoLines} from '../util.js';
export default main16;

const RULE_REGEX = /(?<name>[a-z ]+): (?<range1Min>[0-9]+)-(?<range1Max>[0-9]+) or (?<range2Min>[0-9]+)-(?<range2Max>[0-9]+)/;
const OWN_TICKET_LABEL = 'your ticket:';
const OTHER_TICKETS_LABEL = 'nearby tickets:';
const PART2_RULE_PATTERN = /^departure/;

class Rule {
    constructor(name, ranges) {
        this.name = name;
        this.ranges = ranges;
    }

    isValidForAnyRange(number) {
        for (const [min, max] of this.ranges) {
            if (min <= number && number <= max) {
                return true;
            }
        }
        return false;
    }
}

function main16(input) {
    const lines = splitIntoLines(input);
    const ownTicketLabelIndex = lines.indexOf(OWN_TICKET_LABEL);
    const otherTicketsLabelIndex = lines.indexOf(OTHER_TICKETS_LABEL);
    if (ownTicketLabelIndex < 0 || otherTicketsLabelIndex < 0) {
        throw new Error('Invalid input');
    }
    const rules = parseRules(lines.slice(0, ownTicketLabelIndex));
    const ownTicket = lines[ownTicketLabelIndex + 1].split(',').map(
        str => Number(str));
    const otherTickets = lines.slice(otherTicketsLabelIndex + 1).map(
        arr => arr.split(',').map(str => Number(str)));

    const part1Solution = calculateScanningErrorRate(rules, otherTickets);
    const ruleMap = matchRules(rules, part1Solution.validTickets);
    const part2Solution = multiplyFields(ruleMap, ownTicket,
                                         PART2_RULE_PATTERN);

    return {'Part 1': part1Solution.scanningErrorRate,
            'Part 2': part2Solution};
}

function parseRules(rulesArrays) {
    const rulesObjects = [];
    for (const rule of rulesArrays) {
        const match = rule.match(RULE_REGEX);
        if (match === null) {
            console.log('Invalid rule', rule);
            continue;
        }
        const newRuleObject = new Rule(
            match.groups.name,
            [[match.groups.range1Min, match.groups.range1Max],
             [match.groups.range2Min, match.groups.range2Max]]);
        rulesObjects.push(newRuleObject);
    }
    return rulesObjects;
}

function calculateScanningErrorRate(rules, tickets) {
    let errorRate = 0;
    const validTickets = [];
    for (const ticket of tickets) {
        let ticketValid = true;
        for (const number of ticket) {
            const validForAny = rules.some(
                rule => rule.isValidForAnyRange(number));
            if (!validForAny) {
                errorRate += number;
                ticketValid = false;
            }
        }
        if (ticketValid) {
            validTickets.push(ticket);
        }
    }
    return {scanningErrorRate: errorRate, validTickets: validTickets};
}

function matchRules(rules, validTickets) {
    const fields = [...Array(validTickets[0].length).keys()];
    const ruleMap = new Map();
    // Figure out which fields each rule could match
    for (const rule of rules) {
        const matchingFields = fields.filter(fieldNumber => doesRuleMatchField(
            rule, validTickets, fieldNumber));
        console.log('Rule', rule, 'matches fields', matchingFields);
        ruleMap.set(rule, matchingFields);
    }
    // Find a rule that can only match one field and eliminate that
    // field from the other rules.
    while (Array.from(ruleMap.values()).some(Array.isArray)) {
        let fieldToEliminate = undefined;
        for (const [rule, matchingFields] of ruleMap.entries()) {
            if (Array.isArray(matchingFields) && matchingFields.length === 1) {
                fieldToEliminate = matchingFields[0];
                // Set the value to the number itself rather than an array
                // so we know we've taken care of this one.
                ruleMap.set(rule, fieldToEliminate);
                console.log('Rule', rule, 'can only match field',
                            fieldToEliminate);
                break;
            }
        }
        for (const [rule, matchingFields] of ruleMap.entries()) {
            if (!Array.isArray(matchingFields)) {
                continue;
            }
            ruleMap.set(rule, matchingFields.filter(
                fieldNumber => fieldNumber !== fieldToEliminate));
        }
    }
    return ruleMap;
}

function doesRuleMatchField(rule, tickets, fieldNumber) {
    for (const ticket of tickets) {
        if (!rule.isValidForAnyRange(ticket[fieldNumber])) {
            return false;
        }
    }
    return true;
}

function multiplyFields(ruleMap, ownTicket, rulePattern) {
    let product = 1;
    for (const [rule, field] of ruleMap) {
        if (rulePattern.test(rule.name)) {
            product *= ownTicket[field];
        }
    }
    return product;
}
