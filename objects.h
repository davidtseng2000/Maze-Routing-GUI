#ifndef _OBJECTS_H
#define _OBJECTS_H

using namespace std;

#include <vector>
#include <unordered_map>

class Cell{
public:
    int x, y;
    bool visited = false;
    bool is_obstacle = false;

    Cell *parent = nullptr;

    bool is_start = false, is_end = false;
    int path_id = -1;
};

class Grid{
public:
    vector<vector<Cell>> grid;

    // net_points records the starting and ending points
    unordered_map<int, pair<Cell*, Cell*>> net_points;

    int M = 0, N = 0;

    Grid(int m, int n) : grid(m, vector<Cell>(n)), M(m), N(n) {
        for (int i = 0; i < M; ++i)
            for (int j = 0; j < N; ++j)
                grid[i][j].x = i, grid[i][j].y = j;
    }

    Grid(){}
    ~Grid(){}
    
    void print(int);
    vector<Cell*> get_neighbors(Cell*);
};

class Router{
public:        
    
    // unordered_map<int,int> route(Grid& g); // it returns a map of form "route_id => steps"
    unordered_map<int,int> route(Grid& g, bool use_astar = false);
    int bfs(Grid& g, Cell* start, Cell* end);
    int astar(Grid& g, Cell* start, Cell* end);
    int backtrace(Grid& g, int r_id);
    void reset_grid_state(Grid& g); // reset the state of the cell, including "parent" and "visited"
};

#endif
