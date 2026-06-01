#include <iostream>
#include <algorithm>
#include <vector>
#include <emscripten/bind.h>

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
    Graph(const int w, const int h) : width(w), height(h) {
        Instantiate(width, height);
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
    };



EMSCRIPTEN_BINDINGS(game_module) {
    class_<Graph>("Graph")
        .constructor<int, int>()
        .function("Move", &Graph::Move)
        .function("CheckForWin", &Graph::CheckForWin)
        .function("GetCellState", &Graph::GetCellState);
}