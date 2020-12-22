import {splitIntoLines} from '../util.js';
export default main22;

class Result {
    constructor(winner, deck) {
        this.winner = winner;
        this.deck = deck;
    }
}

function main22(input) {
    const lines = splitIntoLines(input);
    const deck1 = [];
    const deck2 = [];
    let activeDeck = deck1;
    for (const line of lines.slice(1)) {
        const number = Number(line);
        if (!Number.isNaN(number)) {
            activeDeck.push(number);
        } else {
            activeDeck = deck2;
        }
    }
    console.log('Deck 1', deck1);
    console.log('Deck 2', deck2);

    const resultPart1 = runGame(deck1.slice(), deck2.slice(), false, new Map());
    const scorePart1 = calculateScore(resultPart1.deck);

    const resultPart2 = runGame(deck1.slice(), deck2.slice(), true, new Map());
    const scorePart2 = calculateScore(resultPart2.deck);

    return {'Part 1': scorePart1, 'Part 2': scorePart2};
}

function runGame(deck1, deck2, recursive, globalMemo) {
    const startingState = decksToState(deck1, deck2);
    if (globalMemo.has(startingState)) {
        const result = globalMemo.get(startingState);
        console.log('Already know that winner for', startingState, 
                    'is', result.winner);
        return result;
    }
    const seenStatesThisGame = new Set();
    while (deck1.length > 0 && deck2.length > 0) {
        const currentState = decksToState(deck1, deck2);
        if (seenStatesThisGame.has(currentState)) {
            console.log('Player 1 wins to prevent looping forever');
            const result = new Result(1, deck1);
            globalMemo.set(startingState, result);
            return result;
        }
        seenStatesThisGame.add(currentState);

        singleRound(deck1, deck2, recursive, globalMemo);
    }
    if (deck1.length > 0) {
        console.log('Player 1 wins game');
        const result = new Result(1, deck1);
        globalMemo.set(startingState, result);
        return result;
    }
    console.log('Player 2 wins game');
    const result = new Result(2, deck2);
    globalMemo.set(startingState, result);
    return result;
}

function singleRound(deck1, deck2, recursive, globalMemo) {
    const card1 = deck1.shift();
    const card2 = deck2.shift();
    let player1Wins = undefined;
    if (recursive && card1 <= deck1.length && card2 <= deck2.length) {
        const result = runGame(deck1.slice(0, card1), deck2.slice(0, card2),
                               recursive, globalMemo);
        player1Wins = (result.winner === 1);
    } else {
        player1Wins = (card1 > card2);
    }
    if (player1Wins) {
        deck1.push(card1);
        deck1.push(card2);
    } else {
        deck2.push(card2);
        deck2.push(card1);
    }
}

function calculateScore(deck) {
    let score = 0;
    for (let i = 0; i < deck.length; ++i) {
        score += deck[i] * (deck.length - i);
    }
    return score;
}

function decksToState(deck1, deck2) {
    // Convert decks into strings so they can be used in sets and maps
    return deck1.join(',') + '|' + deck2.join(',');
}
