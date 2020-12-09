'use strict';

import {readFile} from './util.js';

const DAY_NUMBER = 9;
const NUM_STARS = DAY_NUMBER * 2;

function main() {
    document.getElementById('favicon-link').href = generateFavicon(NUM_STARS);

    for (let day = 1; day <= DAY_NUMBER; ++day) {
        appendDay(document.body, day);
    }
}

function generateFavicon(numStars) {
    console.log('Generating favicon...');
    const canvas = document.createElement('canvas');
    canvas.width = 16;
    canvas.height = 16;
    // Use colors from AoC theme
    const background = [10, 14, 37];
    const foreground = [255, 255, 85];
    const context = canvas.getContext('2d');
    const imageData = context.createImageData(canvas.width, canvas.height);
    for (let i = 0; i < canvas.width * canvas.height; ++i) {
        let color = background;
        if (numStars > 0 && (i % canvas.width) % 2
            && (i % canvas.width) < canvas.width - 1
            && (Math.floor(i / canvas.width) % 2)) {
            // Fill in odd-numbered pixels as foreground
            color = foreground;
            --numStars;
        }
        [0, 1, 2].forEach(function(offset) {
            imageData.data[i * 4 + offset] = color[offset];
        });
        imageData.data[i * 4 + 3] = 255;    // Alpha
    }
    context.putImageData(imageData, 0, 0);
    console.log('Generated favicon');
    return canvas.toDataURL();
}

function appendDay(parentNode, dayNumber) {
    var dayString = String(dayNumber);
    if (dayNumber < 10) {
        dayString = '0' + dayString;
    }
    const newPar = document.createElement('p');
    newPar.appendChild(document.createTextNode('Day ' + dayString));

    const button = document.createElement('button');
    button.innerHTML = 'Solve';
    const span = document.createElement('span');
    button.onclick = () => solveDay(dayString, span);

    newPar.appendChild(button);
    newPar.appendChild(span);
    document.body.appendChild(newPar);
}

function solveDay(dayString, solutionSpan) {
    console.log('Solving day', dayString);
    const modulePath = `./day${dayString}/solution${dayString}.js`;
    const inputPath = `./day${dayString}/input${dayString}.txt`;
    console.log('Module', modulePath, ', input', inputPath);
    import(modulePath).then((solver) => {
        readFile(inputPath, (fileContents) => {
            const result = solver.default(fileContents);
            solutionSpan.innerHTML = JSON.stringify(result);
            console.log('Solved day', dayString);
        });
    });
}

document.body.onload = main;
