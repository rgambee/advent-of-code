'use strict';

import {main01} from './day01/solution01.js';

function solve01() {
    const result = main01('./day01/input01.txt');
    document.getElementById('solution01').innerHTML = JSON.stringify(result);
}

document.getElementById('button01').onclick = solve01;
