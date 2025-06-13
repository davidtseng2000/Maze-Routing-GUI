#ifndef _ILP_SOLVER_H
#define _ILP_SOLVER_H

#include <vector>
#include <map>
#include <set>
#include <gurobi_c++.h>
#include "objects.h"
#include "path.h"

class ILPSolver {
public:
    // 輸入所有可能的路徑，返回最佳的不衝突路徑集合
    std::vector<Path> solve(const std::vector<Path>& all_paths);

    // Set solver parameters
    void set_time_limit(double seconds) { time_limit = seconds; }
    void set_thread_count(int count) { thread_count = count; }

private:    
    // Building ILP Model
    void build_model(GRBModel& model, 
                    const std::vector<Path>& all_paths,
                    std::vector<GRBVar>& y_vars,  // 路徑選擇變數
                    std::map<std::pair<int, int>, std::vector<std::pair<GRBVar, int>>>& x_vars);  // 格子使用變數

    // Solver parameters
    double time_limit = 30.0;
    int thread_count = 1;
};

#endif 