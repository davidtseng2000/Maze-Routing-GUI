#include <iostream>
#include <vector>
#include <queue>
#include "objects.h"

using namespace std;

vector<Cell*> Grid::get_neighbors(Cell* c){
    
    vector<Cell*> res;
    
    int i = c->x;
    int j = c->y;
    
    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int dir = 0; dir < 4; ++dir) {

        int ni = i + dx[dir];
        int nj = j + dy[dir];
        
        if (ni >= 0 && ni < M && nj >= 0 && nj < N && !grid[ni][nj].is_obstacle) {
            res.push_back(&grid[ni][nj]);
        }        
    }
    return res;
};


void Grid::print(int mode = 0) {
    if(mode == 0){
        cout << "Print the original maze:\n";
    }
    else if(mode == 1){
        cout << "Print the solution to the maze routing problem:\n";
    }

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            const Cell& c = grid[i][j];
            if (c.is_obstacle) cout << '#';
            else if (c.path_id != -1) cout << c.path_id;
            else cout << '.';
        }
        cout << endl;
    }
    cout << endl;
}

void Router::reset_grid_state(Grid& g){
    for (auto& row : g.grid) {
        for (auto& c : row) {            
            c.visited = false;
            c.parent = nullptr;
        }
    }
}

int Router::bfs(Grid& g, Cell* start, Cell* end) {
    queue<Cell*> q;
    q.push(start);
    start->visited = true;
    while (!q.empty()) {
        Cell* cur = q.front();
        q.pop();
        if (cur == end) break;
        for (Cell* n : g.get_neighbors(cur)) {
            if (!n->visited && (n->path_id == -1 || n->is_end)) {
                n->visited = true;
                n->parent = cur;
                q.push(n);
            }
        }
    }
    return backtrace(g, start->path_id);
}


int Router::astar(Grid& g, Cell* start, Cell* end) {
    auto heuristic = [end](Cell* a) {
        return abs(a->x - end->x) + abs(a->y - end->y);
    };

    using PQElem = pair<int, Cell*>;
    priority_queue<PQElem, vector<PQElem>, greater<PQElem>> pq;

    pq.emplace(heuristic(start), start);
    start->visited = true;

    unordered_map<Cell*, int> g_score;
    g_score[start] = 0;

    while (!pq.empty()) {
        Cell* cur = pq.top().second;
        pq.pop();
        if (cur == end) break;

        for (Cell* n : g.get_neighbors(cur)) {
            if (n->is_obstacle || (n->path_id != -1 && !n->is_end)) continue;
            int tentative_g = g_score[cur] + 1;
            if (!g_score.count(n) || tentative_g < g_score[n]) {
                g_score[n] = tentative_g;
                int f = tentative_g + heuristic(n);
                pq.emplace(f, n);
                n->parent = cur;
                n->visited = true;
            }
        }
    }

    return backtrace(g, start->path_id);
}

unordered_map<int,int> Router::route(Grid& g, bool use_astar){
    unordered_map<int,int> id_to_steps;
    for(const auto& [id, endpoints] : g.net_points){
        reset_grid_state(g);
        Cell *start = endpoints.first, *end = endpoints.second;
        int steps = use_astar ? astar(g, start, end) : bfs(g, start, end);
        id_to_steps[id] = steps;
    }    
    return id_to_steps;    
}


int Router::backtrace(Grid& g, int r_id){
    int steps = 1;
    Cell *end = g.net_points[r_id].second; // endpoint
    Cell *start = g.net_points[r_id].first; // startpoint
    Cell *cur = end;

    while (cur != nullptr && cur != start) {
        cur->path_id = r_id;
        cur = cur->parent;
        steps++;
    }

    if(cur == start) return steps;
    else return -1;
}