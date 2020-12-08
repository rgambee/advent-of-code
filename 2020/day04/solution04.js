import {splitIntoLines} from '../util.js';
export default main04;

function main04(input) {
    const fieldValidators = {
        byr: isByrValid,
        iyr: isIyrValid,
        eyr: isEyrValid,
        hgt: isHgtValid,
        hcl: isHclValid,
        ecl: isEclValid,
        pid: isPidValid
    };

    const lines = splitIntoLines(input, false);
    let validCountPart1 = 0;
    let validCountPart2 = 0;
    let passport = {};

    for (let line of lines) {
        if (line.length == 0) {
            console.log('Passport: ', passport);
            if (isPassportValidPart1(passport, Object.keys(fieldValidators))) {
                ++validCountPart1;
                if (isPassportValidPart2(passport, fieldValidators)) {
                    ++validCountPart2;
                }
            } else {
                console.log('Invalid: missing field(s)');
            }
            passport = {};
        } else {
            updatePassport(passport, line);
        }
    }

    return {'Part 1': validCountPart1, 'Part 2': validCountPart2};
}

function updatePassport(passport, line) {
    for (let pair of line.split(' ')) {
        const [field, value] = pair.split(':');
        passport[field] = value;
    }
}

function isPassportValidPart1(passport, fields) {
    return fields.filter(field => field in passport).length == fields.length;
}

function isPassportValidPart2(passport, fieldValidators) {
    if (!isPassportValidPart1(passport, Object.keys(fieldValidators))) {
        return false;
    }
    for (let field in fieldValidators) {
        if (!fieldValidators[field](passport[field])) {
            console.log('Field', field, 'is invalid');
            return false;
        }
    }
    return true;
}

function isByrValid(byr) {
    return 1920 <= Number(byr) && Number(byr) <= 2002;
}

function isIyrValid(iyr) {
    return 2010 <= Number(iyr) && Number(iyr) <= 2020;
}

function isEyrValid(eyr) {
    return 2020 <= Number(eyr) && Number(eyr) <= 2030;
}

function isHgtValid(hgt) {
    if (hgt.endsWith('cm')) {
        const height = Number(hgt.slice(0, -2));
        return 150 <= height && height <= 193;
    } else if (hgt.endsWith('in')) {
        const height = Number(hgt.slice(0, -2));
        return 59 <= height && height <= 76;
    }
    return false;
}

function isHclValid(hcl) {
    return Boolean(hcl.match(/^#[0-9a-f]{6}$/));
}

function isEclValid(ecl) {
    return ['amb', 'blu', 'brn', 'gry', 'grn', 'hzl', 'oth'].includes(ecl);
}

function isPidValid(pid) {
    return pid.length == 9 && !Number.isNaN(Number(pid));
}
