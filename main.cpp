#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include "objects.h"
#include "utils.h"
#include "draw.h"

using namespace std;

// Declare the global variable
extern map<int, int> id_to_steps;

void InputFormatError(){
    cout << "Input format error!\n";
    cout << "Correct format:\n";
    cout << "./main INPUT_MAZE.txt [--print] [--no-gui] [--astar] [--ilp] [--max-iter N] [--time-limit T] [--threads N]\n";
    cout << "  --max-iter N    : Maximum iterations for ILP solver (default: 1)\n";
    cout << "  --time-limit T  : Time limit in seconds for ILP solver (default: 30)\n";
    cout << "  --threads N     : Number of threads for ILP solver (default: 1)\n";
    exit(1);
}

int main(int argc, char** argv) {
    cout << "Starting program..." << endl;
    
    if (argc < 2 || argc > 9) {
        InputFormatError();
    }

    // Input Parameters
    string input_file = argv[1];
    bool enable_print = false;
    bool enable_gui = true;
    bool use_astar = false;
    bool use_ilp = false;
    int max_iteration = 1;
    double time_limit = 30.0;
    int thread_count = 1;

    cout << "Parsing command line arguments..." << endl;
    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--print") {
            enable_print = true;
            cout << "Print mode enabled" << endl;
        } 
        else if (arg == "--no-gui") {
            enable_gui = false;
            if(enable_print)
                cout << "GUI disabled" << endl;
        } 
        else if (arg == "--astar") {
            use_astar = true;
            if(enable_print)
                cout << "A* algorithm enabled" << endl;
        }
        else if (arg == "--ilp") {
            use_ilp = true;
            if(enable_print)
                cout << "ILP algorithm enabled" << endl;
        }
        else if (arg == "--max-iter" && i + 1 < argc) {
            max_iteration = stoi(argv[++i]);
            if(enable_print)
                cout << "Max iterations set to: " << max_iteration << endl;
        }
        else if (arg == "--time-limit" && i + 1 < argc) {
            time_limit = stod(argv[++i]);
            if(enable_print)
                cout << "Time limit set to: " << time_limit << " seconds" << endl;
        }
        else if (arg == "--threads" && i + 1 < argc) {
            thread_count = stoi(argv[++i]);
            if(enable_print)
                cout << "Thread count set to: " << thread_count << endl;
        }
        else {
            cout << "Unknown argument: " << arg << endl;
            InputFormatError();
        }
    }

    // Reading maze
    if(enable_print)
        cout << "Reading maze from file: " << input_file << endl;
    Grid g = read_maze(argv[1]);
    // if (enable_print) {
    //     cout << "Printing original maze:" << endl;
    //     g.print(0);
    // }

    Router r;

    if(enable_print)
        cout << "Starting routing..." << endl;

    // Store routing results in the global id_to_steps
    if (use_ilp) {
        if(enable_print)
            cout << "Using ILP algorithm for routing" << endl;
        id_to_steps = r.route_with_ilp(g, max_iteration, time_limit, thread_count);
    } 
    else {
        if(enable_print)
            cout << "Using " << (use_astar ? "A*" : "BFS") << " algorithm for routing" << endl;
        id_to_steps = r.route(g, use_astar);
    }

    if(enable_print){

        cout << "\nRouting results:" << endl;

        for (const auto& [id, steps] : id_to_steps) {
            if (steps == -1)
                cout << "Routing failed for net_id " << id << endl;
            else
                cout << "route id: " << id << " => steps: " << steps << endl;
        }
        cout << endl;

        // cout << "Printing routed maze:" << endl;
        // g.print(1);
    }


    if (enable_gui) {
        try {
            if(enable_print)
                cout << "Initializing GUI..." << endl;
            // Building SFML window
            // -> user's screen size
            sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
            int screenW = desktop.width;
            int screenH = desktop.height;
            // -> control windows size
            int cellSize = max(min((screenW * 2/3) / g.N, (screenH * 2/3) / g.M), 4);
            int windowWidth = g.N * cellSize;
            int windowHeight = g.M * cellSize;
            int buttonPanelHeight = 100;
            float PanelHeightRate = (float) buttonPanelHeight / (float) (buttonPanelHeight + windowHeight);
            
            if(enable_print)
                cout << "Creating window with size: " << windowWidth << "x" << (windowHeight + buttonPanelHeight) << endl;
            
            sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight + buttonPanelHeight), "Maze Routing");
            if (!window.isOpen()) {
                cout << "Failed to create window!" << endl;
                return 1;
            }

            // Update window title with routing progress
            int total_routes = g.net_points.size();
            int completed_routes = 0;
            for (const auto& [id, steps] : id_to_steps) {
                if (steps != -1) completed_routes++;
            }
            window.setTitle("Maze Routing - " + std::to_string(completed_routes) + "/" + std::to_string(total_routes) + " routes found!");

            // Initialize SFML Window
            window.clear();
            renderMaze(g, window, cellSize);
            window.display();
           
            if(enable_print)
                cout << "Window created successfully. Starting main loop..." << endl;
            
            // Main Loop        
            while (window.isOpen()) {
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed)
                        window.close();
                    else if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.button == sf::Mouse::Left) {
                            // Convert mouse position to world coordinates
                            sf::Vector2f worldPos = window.mapPixelToCoords(
                                sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                            );
                            // Handle button clicks
                            handleButtonClick(worldPos, window, PanelHeightRate);
                        }
                    }          
                }

                window.clear();
                renderMaze(g, window, cellSize);
                window.display();
            }
        } 
        catch (const std::exception& e) {
            cout << "Error creating window: " << e.what() << endl;
            return 1;
        }
    }

    if(enable_print)
        cout << "Program finished successfully." << endl;
    return 0;
}
