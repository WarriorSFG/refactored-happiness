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
            document.getElementById('status').innerText = `Player ${currentPlayer} Wins!`;
            document.getElementById('board').style.pointerEvents = 'none'; // Lock board
        } else {
            currentPlayer = currentPlayer === 'X' ? 'O' : 'X';
            document.getElementById('status').innerText = `Player ${currentPlayer}'s Turn`;
        }
        renderBoard();
    }
}

function renderBoard() {
    const boardDiv = document.getElementById('board');
    boardDiv.innerHTML = '';

    for (let y = 0; y < height; y++) {
        const rowDiv = document.createElement('div');
        rowDiv.className = 'row';
                
        rowDiv.style.marginLeft = `${y * 17}px`; 

        for (let x = 0; x < width; x++) {
            const cellDiv = document.createElement('div');
            cellDiv.className = 'cell';
                    
            const state = gameBoard.GetCellState(x, y);
            cellDiv.innerText = state !== '.' ? state : '';
                    
            if (state === '.') {
                cellDiv.onclick = () => handleCellClick(x, y);
            } else {
                cellDiv.style.color = state === 'X' ? '#d9534f' : '#5bc0de';
                cellDiv.style.fontWeight = 'bold';
            }

            rowDiv.appendChild(cellDiv);
        }
        boardDiv.appendChild(rowDiv);
    }
}