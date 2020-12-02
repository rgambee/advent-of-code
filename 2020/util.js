export {readFile, splitIntoLines};

function readFile(fileName) {
    console.log('Reading from ', fileName);
    var contents;
    const request = new XMLHttpRequest();
    request.open('GET', fileName, false);
    request.onload = function () {
        if (request.status === 200) {
            contents = request.responseText;
        } else {
            console.log('Failed to load file ' + fileName);
            console.log(request.status);
            console.log(request.statusText);
        }
    };

    request.send();
    return contents;
}

function splitIntoLines(fileContents) {
    let lines = fileContents.split('\n');
    lines = lines.filter(line => line.length > 0);
    return lines;
}
