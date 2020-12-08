export {readFile, splitIntoLines, setIntersection};

function readFile(fileName, callback) {
    console.log('Reading from ', fileName);
    const request = new XMLHttpRequest();
    request.open('GET', fileName);
    request.onload = function () {
        if (request.status === 200) {
            callback(request.responseText);
        } else {
            console.log('Failed to load file ' + fileName);
            console.log(request.status);
            console.log(request.statusText);
        }
    };

    request.send();
}

function splitIntoLines(fileContents, stripEmptyLines=true) {
    let lines = fileContents.split('\n');
    if (stripEmptyLines) {
        lines = lines.filter(line => line.length > 0);
    }
    return lines;
}

function setIntersection(setA, setB) {
    const intersection = new Set();
    for (let item of setA) {
        if (setB.has(item)) {
            intersection.add(item);
        }
    }
    return intersection;
}
