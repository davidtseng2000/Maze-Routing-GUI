🌐 Available Languages: [English](README.md) | [繁體中文](README.zh-TW.md)

# Maze Routing GUI

A maze routing visualization application developed using the SFML graphics library. Includes three different algorithms: BFS, A*, and ILP.

## ✨ Features

- Special ILP Algorithm Implementation

- Maze routing visualization
<br><br><img src="./img/img1.jpg" width="50%">

- Interactive path hover effects with routing information
<br><br><img src="./img/img2.jpg" width="50%">

- Support for downloading maze screenshots
- Support for downloading routing results

## 📊 ILP Algorithm

Assuming we start with N Nets to route, we initialize a set `remaining_nets`. Users can set the number of iterations using the `--max-iter` parameter. In each iteration:

1. For each Net in `remaining_nets`, we use BFS to find possible routes. The routes found for different Nets may conflict (share the same cell).
2. Among the routes found in the previous step, we use the ILP algorithm to find "the maximum number of routes that can coexist without conflicts."
3. We remove these conflict-free routes from `remaining_nets`, indicating that we have determined their paths. These paths cannot conflict with future routes.
4. If there are still elements in `remaining_nets` and we haven't reached the `--max-iter` iteration, we return to step 1 and continue.

## 💻 System Requirements

- C++ compiler
- SFML 2.x or higher
- Arial font file (arial.ttf)
- Gurobi Optimizer 10.0 or higher
  - Download and install from [Gurobi website](https://www.gurobi.com/downloads/)
  - Valid Gurobi license required (free academic license available)

## 🛠️ Compilation Instructions

### Using g++

```bash
# Basic compilation
g++ -c main.cpp draw.cpp -I/path/to/SFML/include -I/path/to/gurobi/include
g++ main.o draw.o -o main -L/path/to/SFML/lib -L/path/to/gurobi/lib -lsfml-graphics -lsfml-window -lsfml-system -lgurobi_c++ -lgurobi100

# Using C++17 standard
g++ -std=c++17 -c main.cpp draw.cpp -I/path/to/SFML/include -I/path/to/gurobi/include
g++ main.o draw.o -o main -L/path/to/SFML/lib -L/path/to/gurobi/lib -lsfml-graphics -lsfml-window -lsfml-system -lgurobi_c++ -lgurobi100
```

### Using Makefile

1.Ensure Makefile exists in the directory

2.Set Gurobi paths in Makefile:
   ```makefile
   GUROBI_DIR = /path/to/gurobi
   GUROBI_INCLUDE = $(GUROBI_DIR)/include
   GUROBI_LIB = $(GUROBI_DIR)/lib
   GUROBI_LIBS = -lgurobi_c++ -lgurobi100
   ```

3.Run the following commands:
```bash
make
# or
make all
```

Clean compilation files:
```bash
make clean
```

## 🚀 Execution Commands

```bash
# Basic execution (with GUI)
./main INPUT_MAZE.txt

# Using A* algorithm
./main INPUT_MAZE.txt --astar

# Using ILP algorithm
./main INPUT_MAZE.txt --ilp [--max-iter N] [--time-limit T] [--threads T]

  * --max-iter: Number of iterations (default: 1)
  * --time-limit: ILP Solver time limit in seconds (default: 30)
  * --threads: Number of ILP Solver threads

# Display results only, no GUI
./main INPUT_MAZE.txt --no-gui

# Print detailed information in terminal
./main INPUT_MAZE.txt --print

# Example: Using multiple options
./main INPUT_MAZE.txt --ilp --max-iter 3 --print
```

### Available Parameters

- `--print`: Print detailed execution information in the terminal
- `--no-gui`: Disable GUI, output results only
- `--astar`: Use A* algorithm for path finding
- `--ilp`: Use ILP algorithm for path finding
  - `--max-iter N`: Set number of ILP iterations
  - `--time-limit T`: Set ILP solver time limit in seconds
  - `--threads T`: Set number of ILP solver threads

## 📝 INPUT_MAZE Format

The input file must follow this format:

1.First line: Two integers M N, representing the number of rows and columns

2.Next M lines: Each line contains N characters representing the maze layout
   - `.`: Empty space
   - `#`: Obstacle
   - `S1`, `S2`, ...: Start points (number after S indicates path ID)
   - `E1`, `E2`, ...: End points (number after E indicates path ID)

### Input Example

```
5 7
S1 . . # . . .
# # . # . # .
S2 # . . . # .
. # # # # # .
. . . E2 . . E1
```

Description:
- Maze size is 5 rows × 7 columns
- Two paths to find:
  - Path 1: from S1 to E1
  - Path 2: from S2 to E2
- `#` represents obstacles that paths cannot pass through
- `.` represents empty spaces that paths can pass through

## 📦 Output Files

- `maze_screenshot.png`: Screenshot of the maze
- `routing_results.txt`: Contains detailed information about all paths
  - Successful paths: Shows path ID and number of steps
  - Failed paths: Marked as failed

### Output Examples

#### routing_results.txt example:
```
route id: 1 => steps: 15
route id: 2 => steps: 23
Routing failed for net_id 3
route id: 4 => steps: 18
```

#### Terminal output example (using --print option):
```
Routing failed for net_id 1
route id: 2 => steps: 3
```

## 🎨 Color Guide

⚪️ White: Empty space

⚫️ Black: Obstacle

🔵 Blue: Start point

🔴 Red: End point

🟢 Green: Path

🟣 Purple: Currently hovered path

## ⚠️ Notes

- Ensure arial.ttf font file is in the program directory
- Routing result files will overwrite existing files with the same name
- Make sure SFML library paths are correct during compilation
- If using Makefile, ensure the paths in Makefile are correctly set 