import {splitIntoLines} from '../util.js';
export default main18;

class Node {
    constructor(left, operation, right) {
        this.left = left;
        this.operation = operation;
        this.right = right;
    }

    evaluate() {
        let leftValue = this.left;
        if (typeof leftValue !== 'number') {
            leftValue = leftValue.evaluate();
        }
        let rightValue = this.right;
        if (typeof rightValue !== 'number') {
            rightValue = rightValue.evaluate();
        }
        if (this.operation === '+') {
            return leftValue + rightValue;
        } else if (this.operation === '*') {
            return leftValue * rightValue;
        }
        console.log('Invalid operator', this.operator);
    }
}

function main18(input) {
    const lines = splitIntoLines(input);
    let sumPart1 = 0n;
    let sumPart2 = 0n;
    for (const line of lines) {
        const expression = parseExpression(line.split(''));
        const evaluated = expression.evaluate();
        console.log(evaluated, '=', expression);
        sumPart1 += BigInt(evaluated);
        const expressionAddFirst = parseExpression(line.split(''), true);
        const evaluatedAddFirst = expressionAddFirst.evaluate();
        console.log(evaluatedAddFirst, '=', expressionAddFirst);
        sumPart2 += BigInt(evaluatedAddFirst);
    }

    return {'Part 1': sumPart1.toString(),
            'Part 2': sumPart2.toString()};
}

function parseExpression(expression, additionFirst, returnTokens) {
    if (additionFirst === undefined) {
        additionFirst = false;
    }
    let rootNode = undefined;
    let currentOperation = undefined;
    while (expression.length > 0) {
        const token = expression.shift();
        if (token === ' ') {
            continue;
        }
        if (returnTokens !== undefined && returnTokens.includes(token)) {
            // Done parsing this subexpression, so return to the caller
            if (rootNode === undefined) {
                console.log('Root node undefined at token', token,
                            'of', expression);
            }
            if (currentOperation !== undefined) {
                console.log('Remaining operation', currentOperation,
                            'at end of expression', expression);
            }
            if (token !== ')') {
                // If this token isn't a closed parenthesis, don't
                // actually consume it. Leave it for the caller.
                expression.unshift(token);
            }
            return rootNode;
        }
        if (rootNode === undefined) {
            if (token === '(') {
                // Parse subexpression separately
                rootNode = parseExpression(expression, additionFirst, ')');
            } else if (Number.isNaN(Number(token))) {
                console.log('Invalid initial token', token);
            } else {
                rootNode = Number(token);
            }
        } else if (token === '+' || token === '*') {
            if (token === '+' || !additionFirst) {
                currentOperation = token;
            } else {
                // If we see a * and addition takes precedence,
                // act as if there are parentheses surrounding
                // the subexpressions on either side.
                rootNode = new Node(rootNode, '*',
                                    parseExpression(expression,
                                                    additionFirst, '*)'));
                if (expression[0] !== '*') {
                    // In this case, the subexpression ended at a close
                    // parenthesis, so we need to stop parsing the
                    // current expression and return it.
                    return rootNode;
                }
                currentOperation = undefined;
            }
        } else if (currentOperation === '+' || !additionFirst) {
            if (token === '(') {
                // Parse subexpression separately
                const subExpression = parseExpression(expression,
                                                      additionFirst, ')');
                // Make the current root the left node of the new tree
                // so that things are evaluated in the correct order.
                rootNode = new Node(rootNode, currentOperation, subExpression);
            } else {
                if (Number.isNaN(Number(token))) {
                    console.log('Invalid next token', token);
                }
                rootNode = new Node(rootNode, currentOperation, Number(token));
            }
            currentOperation = undefined;
        }
    }
    if (currentOperation !== undefined) {
        console.log('Remaining operation', currentOperation,
                    'at end of expression', expression);
    }
    return rootNode;
}
