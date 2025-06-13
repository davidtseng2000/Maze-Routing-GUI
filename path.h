#ifndef _PATH_H
#define _PATH_H

#include <vector>
#include <utility>

using namespace std;

struct Path {
    int net_id;
    vector<pair<int, int>> cells;  // (x, y) coordinates
};

#endif 