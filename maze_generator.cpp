#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_set>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

struct Cell {
    int x, y;
    Cell(int a, int b) : x(a), y(b) {}
    bool operator==(const Cell& other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
    template<>
    struct hash<Cell> {
        size_t operator()(const Cell& c) const {
            return hash<int>()(c.x) ^ hash<int>()(c.y);
        }
    };
}

bool bfs_path(Cell start, Cell end, const vector<vector<string>>& maze, vector<Cell>& path) {
    int M = maze.size();
    int N = maze[0].size();
    vector<vector<bool>> visited(M, vector<bool>(N, false));
    queue<pair<Cell, vector<Cell>>> q;
    q.push({start, {start}});
    visited[start.x][start.y] = true;
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    while (!q.empty()) {
        auto [curr, p] = q.front(); q.pop();
        if (curr.x == end.x && curr.y == end.y) {
            path = p;
            return true;
        }
        for (int d = 0; d < 4; ++d) {
            int nx = curr.x + dx[d];
            int ny = curr.y + dy[d];
            if (nx >= 0 && nx < M && ny >= 0 && ny < N &&
                !visited[nx][ny] && maze[nx][ny] != "#") {
                visited[nx][ny] = true;
                auto next = Cell(nx, ny);
                auto next_path = p;
                next_path.push_back(next);
                q.push({next, next_path});
            }
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Usage: ./maze_generator M N net_count obstacle_density\n";
        return 1;
    }

    int M = stoi(argv[1]);
    int N = stoi(argv[2]);
    int net_count = stoi(argv[3]);
    double density = stod(argv[4]);

    srand(time(0));

    vector<vector<string>> maze(M, vector<string>(N, "."));

    // Place obstacles
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            if ((double)rand() / RAND_MAX < density)
                maze[i][j] = "#";

    unordered_set<Cell> used;
    vector<pair<Cell, Cell>> nets;
    int tries = 0;
    while (nets.size() < (size_t)net_count && tries < 5000) {
        tries++;
        int x1 = rand() % M, y1 = rand() % N;
        int x2 = rand() % M, y2 = rand() % N;
        Cell s(x1, y1), e(x2, y2);

        if (s == e || used.count(s) || used.count(e)) continue;

        maze[x1][y1] = "."; maze[x2][y2] = ".";

        vector<Cell> path;
        if (bfs_path(s, e, maze, path)) {
            for (auto& c : path)
                maze[c.x][c.y] = ".";
            used.insert(s);
            used.insert(e);
            nets.emplace_back(s, e);
        }
    }

    for (size_t i = 0; i < nets.size(); ++i) {
        auto [s, e] = nets[i];
        maze[s.x][s.y] = "S" + to_string(i+1);
        maze[e.x][e.y] = "E" + to_string(i+1);
    }

    string filename = "maze_" + to_string(M) + "x" + to_string(N) + ".txt";
    ofstream fout(filename);
    fout << M << " " << N << "\n";
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            fout << " " << maze[i][j];
        }
        fout << "\n";
    }

    cout << "Maze generated and saved to: " << filename << "\n";
    return 0;
}
