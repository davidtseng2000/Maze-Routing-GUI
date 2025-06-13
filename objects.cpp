#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include "objects.h"
#include "path.h"
#include "ilp_solver.h"

using namespace std;

// Basic Functions
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
            else if (c.path_id != -1){
                cout << c.path_id;
            }
            else cout << '.';
        }
        cout << endl;
    }
    cout << endl;
}


// Maze Routing main algorithm (BFS / Lee's algo)
map<int,int> Router::route(Grid& g, bool use_astar){
    map<int,int> id_to_steps;
    for(const auto& [id, endpoints] : g.net_points){
        reset_grid_state(g);
        Cell *start = endpoints.first, *end = endpoints.second;
        int steps = use_astar ? astar(g, start, end) : bfs(g, start, end);
        id_to_steps[id] = steps;
    }    
    return id_to_steps;    
}

int Router::bfs(Grid& g, Cell* start, Cell* end) {
    queue<Cell*> q;
    q.push(start);
    int rid = start->path_id;
    start->visited[rid] = true;
    while (!q.empty()) {
        Cell* cur = q.front();
        q.pop();
        if (cur == end) break;
        for (Cell* n : g.get_neighbors(cur)) {
            // (n->is_end && n->path_id == rid): 此鄰居為現在要找的 route 的 ending point
            // (n->is_space && n->path_id == -1): 此鄰居為一般的可走點 (非 end 也非 start)
            if (!n->visited[rid] && ((n->is_end && n->path_id == rid) || (n->is_space && n->path_id == -1))) {
                n->visited[rid] = true;
                n->parent = cur;
                q.push(n);
            }
        }
    }
    return backtrace(g, rid);
}

int Router::backtrace(Grid& g, int rid){
    Cell *end = g.net_points[rid].second; // endpoint
    Cell *start = g.net_points[rid].first; // startpoint
    Cell *cur = end;

    vector<Cell*> path = {};

    while (cur != nullptr && cur != start) {
        path.push_back(cur);
        cur = cur->parent;
    }

    if(cur == start){
        path.push_back(cur);
        for(auto cell : path) {
            cell->path_id = rid;
        }
        return path.size();
    }
    else return -1;
}

// Heuristic Astar algo
int Router::astar(Grid& g, Cell* start, Cell* end) {
    auto heuristic = [end](Cell* a) {
        return abs(a->x - end->x) + abs(a->y - end->y);
    };

    struct PQElem {
        int f;  // f = g + h
        int h;  // heuristic value
        Cell* cell;
        
        PQElem(int f_val, int h_val, Cell* c) : f(f_val), h(h_val), cell(c) {}
        
        bool operator>(const PQElem& other) const {
            if (f != other.f) return f > other.f;
            if (h != other.h) return h > other.h;
            if (cell->x != other.cell->x) return cell->x > other.cell->x;
            return cell->y > other.cell->y;
        }
    };
    
    priority_queue<PQElem, vector<PQElem>, greater<PQElem>> pq;

    pq.emplace(heuristic(start), heuristic(start), start);

    int rid = start->path_id;
    start->visited[rid] = true;
    

    unordered_map<Cell*, int> g_score;
    g_score[start] = 0;

    while (!pq.empty()) {
        Cell* cur = pq.top().cell;
        pq.pop();
        if (cur == end) break;

        for (Cell* n : g.get_neighbors(cur)) {
            if (!n->visited[rid] && ((n->is_end && n->path_id == rid) || (n->is_space && n->path_id == -1))){
                int tentative_g = g_score[cur] + 1;
                if (!g_score.count(n) || tentative_g < g_score[n]) {
                    g_score[n] = tentative_g;
                    int f = tentative_g + heuristic(n);
                    pq.emplace(f, heuristic(n), n);
                    n->parent = cur;
                    n->visited[rid] = true;
                }
            }
        }
    }

    return backtrace(g, start->path_id);
}


void Router::reset_grid_state(Grid& g){
    for (auto& row : g.grid) {
        for (auto& c : row) {
            for(auto &[k, v] : c.visited)
                v = false;
            c.parent = nullptr;
        }
    }
}

// ILP Algorithm
map<int,int> Router::route_with_ilp(Grid& g, int max_iteration, double time_limit, int thread_count) {
    map<int,int> id_to_steps;
    set<int> remaining_nets;
    
    // Initialize all nets
    for (const auto& [id, _] : g.net_points) {
        remaining_nets.insert(id);
    }
    
    // cout << "Starting ILP routing with " << remaining_nets.size() << " nets" << endl;
    // cout << "Time limit: " << time_limit << " seconds, Thread count: " << thread_count << endl;
    
    ILPSolver solver;
    solver.set_time_limit(time_limit);
    solver.set_thread_count(thread_count);
    
    while (!remaining_nets.empty() && max_iteration) {
                
        // Finding routes for remaining paths.
        vector<Path> all_paths = find_all_paths(g, remaining_nets);
        
        if (all_paths.empty()) {
            // cout << "No more paths found for remaining nets" << endl;
            break;  // No more path can be found.
        }
        
        // cout << "Found " << all_paths.size() << " possible paths" << endl;
        
        // Main ILP to find maximal non-conflicting routes
        vector<Path> selected_paths = solver.solve(all_paths);
        
        if (selected_paths.empty()) {
            // cout << "ILP solver couldn't find any non-conflicting paths" << endl;
            break;  // ILP can't find a route.
        }
        
        // cout << "ILP solver selected " << selected_paths.size() << " paths" << endl;
        
        // Adding the selected routes to grid
        for (const auto& path : selected_paths) {
            apply_path_to_grid(g, path);
            remaining_nets.erase(path.net_id);
            id_to_steps[path.net_id] = path.cells.size();
            // cout << "Applied path for net " << path.net_id << " with " << path.cells.size() << " cells" << endl;
        }
        max_iteration--;
    }
    
    // The nets which can't match. (Can't find a route)
    for (int net_id : remaining_nets) {
        // cout << "Failed to route net " << net_id << endl;
        id_to_steps[net_id] = -1;
    }
    
    // cout << "ILP routing completed. Successfully routed " << (g.net_points.size() - remaining_nets.size()) 
    //      << " out of " << g.net_points.size() << " nets" << endl;
    
    return id_to_steps;
}

Path Router::bfs_ilp(Grid& g, Cell* start, Cell* end) {
    queue<Cell*> q;
    q.push(start);
    int rid = start->path_id;
    start->visited[rid] = true;
    while (!q.empty()) {
        Cell* cur = q.front();
        q.pop();
        if (cur == end) break;
        for (Cell* n : g.get_neighbors(cur)) {
            // (n->is_end && n->path_id == rid): 此鄰居為現在要找的 route 的 ending point
            // (n->is_space && n->path_id == -1): 此鄰居為一般的可走點 (非 end 也非 start)
            if (!n->visited[rid] && ((n->is_end && n->path_id == rid) || (n->is_space && n->path_id == -1))) {
                n->visited[rid] = true;
                n->parent = cur;
                q.push(n);
            }
        }
    }
    return backtrace_ilp(g, rid);    
}

Path Router::backtrace_ilp(Grid& g, int rid){

    Cell *end = g.net_points[rid].second; // endpoint
    Cell *start = g.net_points[rid].first; // startpoint
    Cell *cur = end;

    Path p;
    p.net_id = -1;
    vector<Cell*> path_temp = {};
    vector<pair<int, int>> cells;

    while (cur != nullptr && cur != start) {
        path_temp.push_back(cur);
        cur = cur->parent;
    }

    if(cur == start){
        path_temp.push_back(cur);
        for(auto cell : path_temp) {
            cells.push_back({cell->x, cell->y});
        }
        p.cells = cells;
        p.net_id = rid;
    }   

    return p;
}

// This function works like "bfs", but "conflicts" are acceptable (will be determined which path survives by ILP later)
vector<Path> Router::find_all_paths(Grid& g, const set<int>& target_nets) {

    vector<Path> all_paths;
    
    for (int net_id : target_nets) {
        // cout << "Finding " << net_id << endl;
        reset_grid_state(g);
        Cell* start = g.net_points[net_id].first;
        Cell* end = g.net_points[net_id].second;

        Path p = bfs_ilp(g, start, end);
        // cout << "candidates: ";
        if(p.net_id != -1){
            all_paths.push_back(p);
            // cout << p.net_id << " ";
        }
        // cout << endl;
    }    
    return all_paths;
}

void Router::apply_path_to_grid(Grid& g, const Path& path) {
    for (const auto& [x, y] : path.cells) {
        g.grid[x][y].path_id = path.net_id;
    }
}