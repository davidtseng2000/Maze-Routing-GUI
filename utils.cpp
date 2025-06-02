#include "utils.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;

Grid read_maze(const string& filename) {
    ifstream input;
    input.open(filename);
    if(input.fail()){
        cout << "Cannot read the input file!\n";
        exit(1);
    }

    int m, n;
    input >> m >> n;

    Grid g(m,n);
    string token;

    for(int i = 0; i < m; i++){
        for(int j = 0; j < n; j++){
            input >> token;
            Cell& cell = g.grid[i][j];
            cell.x = i;
            cell.y = j;

            if (token == "#") {
                cell.is_obstacle = true;
            }
            else if (token == ".") {
            }
            else if (token.size() >= 2 && (token[0] == 'S' || token[0] == 'E')) {
                char type = token[0]; // S or E
                int net_id = stoi(token.substr(1));
            
                if (type == 'S') {
                    cell.path_id = net_id;
                    cell.is_start = true;
                    g.net_points[net_id].first = &cell;
                }
                else{ // 'E'
                    cell.path_id = net_id;
                    cell.is_end = true;
                    g.net_points[net_id].second = &cell;
                }
            }
            else {
                cout << "Invalid token in input: " << token << endl;
                exit(1);
            }
        }
    }
    
    for (const auto& [id, pair] : g.net_points) {
        if (pair.first == nullptr || pair.second == nullptr) {
            cout << "Missing S" << id << " or E" << id << "!\n";
            exit(1);
        }
    }    

    input.close();
    return g;
}
