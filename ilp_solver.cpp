#include "ilp_solver.h"
#include "path.h"
#include <algorithm>
#include <map>

void ILPSolver::build_model(GRBModel& model,
                          const std::vector<Path>& all_paths,
                          std::vector<GRBVar>& y_vars,
                          std::map<std::pair<int, int>, std::vector<std::pair<GRBVar, int>>>& x_vars) {
    try {
        // cout << "Building ILP model..." << endl;


        // 創建路徑選擇變數 y_p
        // cout << "Creating path selection variables..." << endl;
        y_vars.resize(all_paths.size());  // 預先分配空間
        for (size_t i = 0; i < all_paths.size(); ++i) {
            y_vars[i] = model.addVar(0.0, 1.0, 1.0, GRB_BINARY, "y_" + std::to_string(all_paths[i].net_id));
        }
        // cout << "Created " << y_vars.size() << " path selection variables" << endl;

        // 創建格子使用變數 x_{ij,p}
        // cout << "Creating cell usage variables..." << endl;
        for (size_t p = 0; p < all_paths.size(); ++p) {
            // cout << "Processing path " << p << " (net_id: " << all_paths[p].net_id << ") with " << all_paths[p].cells.size() << " cells" << endl;
            for (const auto& cell : all_paths[p].cells) {
                x_vars[cell].push_back({model.addVar(0.0, 1.0, 0.0, GRB_BINARY, 
                    "x_" + std::to_string(cell.first) + "_" + std::to_string(cell.second) + "_" + std::to_string(all_paths[p].net_id)),
                    all_paths[p].net_id});
            }
        }
        // cout << "Created cell usage variables" << endl;

        // 添加限制式：同一條路徑上的點必須一起被選中或一起不被選中
        // cout << "Adding path consistency constraints..." << endl;
        int constraint_count = 0;
        for (size_t p = 0; p < all_paths.size(); ++p) {
            // cout << "Adding constraints for path " << p << " (net_id: " << all_paths[p].net_id << ")" << endl;
            for (const auto& cell : all_paths[p].cells) {
                for(const auto& [var, net_id] : x_vars[cell]){
                    if(net_id == all_paths[p].net_id){
                        model.addConstr(var == y_vars[p]);
                        constraint_count++;
                    }
                }              
            }
        }
        // cout << "Added " << constraint_count << " path consistency constraints" << endl;

        // 添加限制式：每個格子最多只能被一條路徑使用
        // cout << "Adding cell usage constraints..." << endl;
        constraint_count = 0;
        for (const auto& [cell, vars] : x_vars) {
            try {
                // cout << "Adding cell usage constraint for cell (" << cell.first << "," << cell.second << ")" << endl;
                GRBLinExpr sum;
                for (const auto& [var, net_id] : vars) {
                    sum += var;
                }
                model.addConstr(sum <= 1);
                constraint_count++;
                // cout << "Added cell usage constraint " << constraint_count << endl;
            } 
            catch (const std::exception& e) {
                cout << "Error adding cell usage constraint for cell (" 
                     << cell.first << "," << cell.second << "): " << e.what() << endl;
                throw;
            }
        }
        // cout << "Added " << constraint_count << " cell usage constraints" << endl;

        // 設置目標函數：最大化選中的路徑數量
        // cout << "Setting objective function..." << endl;
        GRBLinExpr objective;
        for (const auto& var : y_vars) {
            objective += var;
        }
        model.setObjective(objective, GRB_MAXIMIZE);
        // cout << "Set objective function" << endl;

    } 
    catch (GRBException& e) {
        std::cerr << "Error building model: " << e.getErrorCode() << std::endl;
        std::cerr << e.getMessage() << std::endl;
        throw;
    }
}

std::vector<Path> ILPSolver::solve(const std::vector<Path>& all_paths) {
    try {
        // cout << "Initializing Gurobi environment..." << endl;
        GRBEnv env = GRBEnv();
        
        // 設置求解器參數
        env.set(GRB_IntParam_OutputFlag, 0);  // 0: 不顯示求解過程
        env.set(GRB_DoubleParam_TimeLimit, time_limit);  // 設置時間限制
        env.set(GRB_IntParam_Threads, thread_count);  // 設置執行緒數量
        
        // cout << "Creating model..." << endl;
        GRBModel model = GRBModel(env);
        
        std::vector<GRBVar> y_vars;
        std::map<std::pair<int, int>, std::vector<std::pair<GRBVar, int>>> x_vars;
        
        build_model(model, all_paths, y_vars, x_vars);
        
        // cout << "Optimizing model..." << endl;
        model.optimize();
        
        int status = model.get(GRB_IntAttr_Status);
        // cout << "Optimization status: " << status << endl;
        
        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {
            // 收集結果
            std::vector<Path> selected_paths;
            for (size_t i = 0; i < all_paths.size(); ++i) {
                if (y_vars[i].get(GRB_DoubleAttr_X) > 0.5) {
                    selected_paths.push_back(all_paths[i]);
                }
            }
            // cout << "Found " << selected_paths.size() << " non-conflicting paths" << endl;
            return selected_paths;
        } 
        else {
            // cout << "No solution found within time limit" << endl;
            return {};
        }
        
    } 
    catch (GRBException& e) {
        std::cerr << "Error in ILP solver: " << e.getErrorCode() << std::endl;
        std::cerr << e.getMessage() << std::endl;
        return {};
    } 
    catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return {};
    }
} 