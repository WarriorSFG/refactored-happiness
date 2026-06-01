let gameBoard;
let currentPlayer = 'X';
const width = 5;  
const height = 5; 

Module.onRuntimeInitialized = () => {
    gameBoard = new Module.Graph(width, height);
    renderBoard();
};

function handleCellClick(x, y) {
    const moveSuccessful = gameBoard.Move(currentPlayer, x, y);
            
    if (moveSuccessful) {
        if (gameBoard.CheckForWin(currentPlayer)) {
            const statusText = document.getElementById('status');
            statusText.innerText = `Player ${currentPlayer} Wins!`;
            statusText.style.color = currentPlayer === 'X' ? '#ff0000' : '#0000ff';
            document.getElementById('board').style.pointerEvents = 'none'; // Lock board
        } else {
            currentPlayer = currentPlayer === 'X' ? 'O' : 'X';
            const statusText = document.getElementById('status');
            statusText.innerText = `Player ${currentPlayer}'s Turn`;
            statusText.style.color = currentPlayer === 'X' ? '#ff0000' : '#0000ff';
        }
        renderBoard();
    }
}

function renderBoard() {
    const boardDiv = document.getElementById('board');
    boardDiv.innerHTML = '';

    const R = 24; 
    const W = Math.sqrt(3) * R; 
    
    const paddingX = 60;
    const paddingY = 60;
    const svgWidth = paddingX * 2 + (width - 1) * W + (height - 1) * (W / 2) + W;
    const svgHeight = paddingY * 2 + (height - 1) * 1.5 * R + 2 * R;

    const svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
    svg.setAttribute("width", "100%");
    svg.setAttribute("height", "100%");
    svg.setAttribute("viewBox", `0 0 ${svgWidth} ${svgHeight}`);
    boardDiv.appendChild(svg);

    const cellCoords = {};

    function getHexPoints(cx, cy) {
        return [
            [cx, cy - R],        
            [cx + W/2, cy - R/2], 
            [cx + W/2, cy + R/2], 
            [cx, cy + R],        
            [cx - W/2, cy + R/2], 
            [cx - W/2, cy - R/2]  
        ];
    }

    for (let y = 0; y < height; y++) {
        for (let x = 0; x < width; x++) {
            const cy = svgHeight - (paddingY + R + y * 1.5 * R);
            const cx = paddingX + W/2 + x * W + y * (W / 2);

            cellCoords[`${x},${y}`] = getHexPoints(cx, cy);
            const pointsStr = cellCoords[`${x},${y}`].map(p => p.join(",")).join(" ");

            const polygon = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
            polygon.setAttribute("points", pointsStr);
            polygon.setAttribute("class", "hex-cell");

            const state = gameBoard.GetCellState(x, y);

            if (state === '.') {
                polygon.style.fill = "#ffffff";
                polygon.onclick = () => handleCellClick(x, y);
            } else if (state === 'X') {
                polygon.style.fill = "#ff0000";
            } else if (state === 'O') {
                polygon.style.fill = "#0000ff"; 
            }

            svg.appendChild(polygon);
        }
    }

    const topRedPoints = [];
    const bottomRedPoints = [];
    const leftBluePoints = [];
    const rightBluePoints = [];

    for (let x = 0; x < width; x++) {
        const pts = cellCoords[`${x},${height - 1}`];
        if (x === 0) topRedPoints.push(pts[5]);
        topRedPoints.push(pts[0], pts[1]);
    }

    for (let x = 0; x < width; x++) {
        const pts = cellCoords[`${x},0`];
        if (x === 0) bottomRedPoints.push(pts[4]);
        bottomRedPoints.push(pts[3], pts[2]);
    }

    for (let y = 0; y < height; y++) {
        const pts = cellCoords[`0,${y}`];
        if (y === 0) leftBluePoints.push(pts[3]);
        leftBluePoints.push(pts[4], pts[5]);
        if (y === height - 1) leftBluePoints.push(pts[0]);
    }

    for (let y = 0; y < height; y++) {
        const pts = cellCoords[`${width - 1},${y}`];
        if (y === 0) rightBluePoints.push(pts[3]);
        rightBluePoints.push(pts[2], pts[1]);
        if (y === height - 1) rightBluePoints.push(pts[0]);
    }

    drawBorderLine(topRedPoints, "border-red");
    drawBorderLine(bottomRedPoints, "border-red");
    drawBorderLine(leftBluePoints, "border-blue");
    drawBorderLine(rightBluePoints, "border-blue");

    function drawBorderLine(pointsArray, className) {
        const polyline = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
        polyline.setAttribute("points", pointsArray.map(p => p.join(",")).join(" "));
        polyline.setAttribute("class", className);
        svg.appendChild(polyline);
    }

    for (let y = 0; y < height; y++) {
        const pts = cellCoords[`0,${y}`];
        const text = document.createElementNS("http://www.w3.org/2000/svg", "text");
        text.setAttribute("x", pts[5][0] - 15);
        text.setAttribute("y", (pts[5][1] + pts[4][1]) / 2 + 6);
        text.setAttribute("class", "label-blue");
        text.setAttribute("text-anchor", "end");
        text.textContent = y + 1;
        svg.appendChild(text);
    }

    for (let x = 0; x < width; x++) {
        const pts = cellCoords[`${x},0`];
        const text = document.createElementNS("http://www.w3.org/2000/svg", "text");
        text.setAttribute("x", pts[3][0]);
        text.setAttribute("y", pts[3][1] + 25);
        text.setAttribute("class", "label-red");
        text.setAttribute("text-anchor", "middle");
        text.textContent = String.fromCharCode(97 + x); 
        svg.appendChild(text);
    }
}