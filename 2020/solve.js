'use strict';

import {main01} from './day01/solution01.js';
import {main02} from './day02/solution02.js';
import {main03} from './day03/solution03.js';

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
