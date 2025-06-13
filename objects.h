#ifndef _OBJECTS_H
#define _OBJECTS_H

using namespace std;

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include "path.h"

class Cell{
public:
    int x, y;
    unordered_map<int,bool> visited;
    bool is_obstacle = false;

    Cell *parent = nullptr;

    bool is_start = false, is_end = false, is_space = false;
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
    map<int,int> route(Grid& g, bool use_astar = false);
    int bfs(Grid& g, Cell* start, Cell* end);
    int astar(Grid& g, Cell* start, Cell* end);
    int backtrace(Grid& g, int r_id);
    void reset_grid_state(Grid& g);

    // For ILP
    map<int,int> route_with_ilp(Grid& g, int max_iteration = 1, double time_limit = 30.0, int thread_count = 1);
    vector<Path> find_all_paths(Grid& g, const set<int>& target_nets);
    void apply_path_to_grid(Grid& g, const Path& path);
    Path bfs_ilp(Grid& g, Cell* start, Cell* end);
    Path backtrace_ilp(Grid& g, int rid);
};

#endif
