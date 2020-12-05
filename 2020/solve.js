'use strict';

import {main01} from './day01/solution01.js';
import {main02} from './day02/solution02.js';
import {main03} from './day03/solution03.js';
import {main04} from './day04/solution04.js';

function solve01() {
    const result = main01('./day01/input01.txt');
    document.getElementById('solution01').innerHTML = JSON.stringify(result);
}
document.getElementById('button01').onclick = solve01;

function solve02() {
    const result = main02('./day02/input02.txt');
    document.getElementById('solution02').innerHTML = JSON.stringify(result);
}
document.getElementById('button02').onclick = solve02;

function solve03() {
    const result = main03('./day03/input03.txt');
    document.getElementById('solution03').innerHTML = JSON.stringify(result);
}
document.getElementById('button03').onclick = solve03;

function solve04() {
    const result = main04('./day04/input04.txt');
    document.getElementById('solution04').innerHTML = JSON.stringify(result);
}
document.getElementById('button04').onclick = solve04;

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
document.getElementById('favicon-link').href = generateFavicon(8);
