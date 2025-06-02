#include <SFML/Graphics.hpp>
#include <iostream>
#include "objects.h"
#include "utils.h"
#include "draw.h"

using namespace std;

// Declare the global variable
extern std::unordered_map<int, int> id_to_steps;

int main(int argc, char** argv) {
    if (argc < 2 || argc > 5) {
        cout << "Input format error!\n";
        cout << "Correct format:\n";
        cout << "./main INPUT_MAZE.txt [--print] [--no-gui] [--astar]\n";
        exit(1);
    }

    // Input Parameters
    string input_file = argv[1];
    bool enable_print = false;
    bool enable_gui = true;
    bool use_astar = false;

    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--print") {
            enable_print = true;
        } 
        else if (arg == "--no-gui") {
            enable_gui = false;
        } 
        else if (arg == "--astar") {
            use_astar = true;
        }
        else {
            cout << "Unknown argument: " << arg << endl;
            exit(1);
        }
    }

    // Reading maze
    Grid g = read_maze(argv[1]);
    if (enable_print) g.print(0);

    Router r;
    // Store routing results in the global id_to_steps
    ::id_to_steps = r.route(g, use_astar);

    for (const auto& [id, steps] : ::id_to_steps) {
        if (steps == -1)
            cout << "Routing failed for net_id " << id << endl;
        else
            cout << "route id: " << id << " => steps: " << steps << endl;
    }
    cout << endl;
    if (enable_print) g.print(1);

    if (enable_gui) {
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
        sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight + buttonPanelHeight), "Maze Routing");

        // Initialize SFML Window
        window.clear();
        renderMaze(g, window, cellSize);
        window.display();
       
        
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

    return 0;
}
