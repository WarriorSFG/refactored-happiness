#include <iostream>
#include <algorithm>
#include <vector>
#include <random>            
#include <stdexcept>
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace std;
using namespace emscripten;

typedef struct Cell {
    int x, y;
    bool isOccupied;
    char Player; 
} Cell;

class Graph {
private:
    int width, height;
    vector<vector<Cell>> grid;
    
    void Instantiate(const int w, const int h){
        grid.resize(height, vector<Cell>(width));
        for(int i=0;i<height;++i){
            for(int j=0;j<width;++j){
                grid[i][j].x = j;
                grid[i][j].y = i;
                grid[i][j].isOccupied = false;
                grid[i][j].Player = '\0';
            }
        }
    }

    inline bool CheckForOccupied(const int i, const int j){
        if (i < 0 || i >= width || j < 0 || j >= height) return false;
        return grid[j][i].isOccupied;
    }

    bool DFS(int x, int y, char Player, vector<vector<bool>>&Visited, bool isTopToBottom){
        if(x<0 || y< 0 || x>= width || y >= height) return false;
        if(grid[y][x].Player != Player || Visited[y][x]) return false;

        Visited[y][x] = true;

        if(isTopToBottom && y == height-1) return true;
        if(!isTopToBottom && x == width-1) return true;

        int dx[] = {-1,1,0,1,-1,0};
        int dy[] = {0,0,-1,-1,1,1};

        for(int i=0;i<6;++i){
            if(DFS(x + dx[i], y+dy[i], Player,Visited,isTopToBottom)) return true;
        }
        return false;
    }

public:
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    Graph(const int w, const int h) : width(w), height(h) {
        Instantiate(width, height);
    }

    Graph(vector<vector<Cell>> g) : grid(g) {
        height = grid.size();
        width = height > 0 ? grid[0].size() : 0;
    }

    bool Move(string playerStr, int x, int y) {
        if (playerStr.empty()) return false;
        char Player = playerStr[0]; 

        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        
        if(!CheckForOccupied(x,y)){
            grid[y][x].isOccupied = true;
            grid[y][x].Player = Player;
            return true;
        }

        return false;
    }

    bool CheckForWin(string playerStr){
        if (playerStr.empty()) return false;
        char Player = playerStr[0];

        vector<vector<bool>> Visited(height, vector<bool>(width, false));
        bool isTopToBottom = (Player == 'X');

        if(isTopToBottom){
            for (int x=0;x<width;++x){
                if(grid[0][x].Player == 'X' && !Visited[0][x]){
                    if(DFS(x, 0, Player, Visited, true)) return true;
                }
            }
        } else {
            for(int y=0;y<height;++y){
                if(grid[y][0].Player == 'O' && !Visited[y][0]){
                    if(DFS(0,y,Player,Visited,false)) return true;
                }
            }
        }
        return false;
    }

    string GetCellState(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) return "";
        if (grid[y][x].isOccupied) {
            return string(1, grid[y][x].Player); 
        }
        return "."; 
    }

    emscripten::val GetBestMoveAI(string playerStr, int simulations);
};

class Stack {
private:
    vector<pair<int, int>> vec; 
    int topIdx;

public:
    Stack() : topIdx(-1) {}

    void Insert(pair<int, int> entry) {
        topIdx++;
        
        if (topIdx >= vec.size()) {
            vec.resize(topIdx + 1);
        }
        vec[topIdx] = entry;
    }

    pair<int, int> Top() {
        if (topIdx == -1) {
            throw underflow_error("Stack is empty");
        }
        return vec[topIdx];
    }

    pair<int, int> Pop() {
        if (topIdx == -1) {
            return make_pair(-1, -1); 
        }
        pair<int, int> item = vec[topIdx];
        topIdx--;
        return item;
    }

    bool Empty(){
        return topIdx == -1;
    }
};

class Simulation : public Graph {
private:
    vector<pair<int,int>> AvailableMoves;
    char Player;
    random_device rd;
    mt19937 gen{rd()};
public:
    Simulation(const Graph& g, char p) : Graph(g), Player(p) {
        int h = GetHeight();
        int w = GetWidth();
        for(int i=0;i<h;++i){
            for(int j=0;j<w;++j){
                if(const_cast<Simulation*>(this)->GetCellState(j,i) == ".") {
                    AvailableMoves.push_back(make_pair(j,i));
                }
            }
        }
    }

    void Run(){
        std::shuffle(AvailableMoves.begin(), AvailableMoves.end(), gen);
        for(const auto& move : AvailableMoves){
            Move(string(1, Player), move.first, move.second);
            Player = (Player == 'X') ? 'O' : 'X';
        }
    }
};

class MonteCarlo {
    Graph* Grid;
    string Player;
public:
    MonteCarlo(Graph* g, string p) : Grid(g), Player(p) {};

    pair<int,int>GetBestMove(int Simulations){
        if(Simulations < 1) return make_pair(-1,-1);
        
        int h = Grid->GetHeight();
        int w = Grid->GetWidth();
        
        Stack PossibleMoves;

        for(int i=0;i<h;++i){
            for(int j=0;j<w;++j){
                if(Grid->GetCellState(j,i) == ".") PossibleMoves.Insert(make_pair(j,i));
            }
        }

        if(PossibleMoves.Empty()) return make_pair(-1,-1);

        pair<int,int> bestMove = make_pair(-1, -1);
        int maxWins = -1;
        while(!PossibleMoves.Empty()){
            int WinCount = 0;
            pair<int,int> currentMove = PossibleMoves.Pop();

            for(int i=0;i<Simulations;++i){
                Graph SimGrid = *Grid;
                SimGrid.Move(Player, currentMove.first, currentMove.second);
                
                char nextPlayer = (Player[0] == 'X') ? 'O' : 'X';
                Simulation Sim(SimGrid, nextPlayer);
                Sim.Run();
                if(Sim.CheckForWin(Player)) WinCount++;
            }

            if(WinCount > maxWins){
                maxWins = WinCount;
                bestMove = currentMove;
            }
        }

        return bestMove;
    }
};
    
emscripten::val Graph::GetBestMoveAI(string playerStr, int simulations) {
    MonteCarlo mc(this, playerStr);
    pair<int, int> bestMove = mc.GetBestMove(simulations);
    
    emscripten::val result = emscripten::val::array();
    result.call<void>("push", bestMove.first);
    result.call<void>("push", bestMove.second);
    return result;
}

EMSCRIPTEN_BINDINGS(game_module) {
    class_<Graph>("Graph")
        .constructor<int, int>()
        .constructor<vector<vector<Cell>>>()
        .function("Move", &Graph::Move)
        .function("CheckForWin", &Graph::CheckForWin)
        .function("GetCellState", &Graph::GetCellState)
        .function("GetBestMoveAI", &Graph::GetBestMoveAI);
}