import {splitIntoLines} from '../util.js';
export default main25;

const SUBJECT_NUMBER = 7;
const MODULO_BASE = 20201227;

function main25(input) {
    const [cardPublicKey, doorPublicKey] = splitIntoLines(input);

    const cardLoopSize = determineLoopSize(cardPublicKey);
    console.log('Card loop size is', cardLoopSize);
    const doorLoopSize = determineLoopSize(doorPublicKey);
    console.log('Door loop size is', doorLoopSize);

    const privateKey = calculatePrivateKey(doorPublicKey, cardLoopSize);
    console.log('Private key is ', privateKey);
    if (privateKey != calculatePrivateKey(cardPublicKey, doorLoopSize)) {
        console.log('Private keys do not match');
    } else {
        console.log('Private keys match');
    }

    return {'Part 1': privateKey};
}

function determineLoopSize(publicKey) {
    let currentValue = 1;
    let loopSize = 0;
    while (currentValue != publicKey) {
        currentValue = (currentValue * SUBJECT_NUMBER) % MODULO_BASE;
        ++loopSize;
    }
    return loopSize;
}

function calculatePrivateKey(publicKey, loopSize) {
    let key = 1;
    for (let i = 0; i < loopSize; ++i) {
        key = (key * publicKey) % MODULO_BASE;
    }
    return key;
}
