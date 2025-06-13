#include <iostream>
#include "draw.h"
#include <fstream>

// Define the global variable
std::map<int, int> id_to_steps;

// Global variables for hover state
static int hovered_path_id = -1;
static sf::Vector2i mouse_position;

// Button class for download buttons
class Button {
public:
    sf::RectangleShape shape;
    sf::Text text;
    bool isHovered = false;

    Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& label, const sf::Font& font) {
        shape.setPosition(position);
        shape.setSize(size);
        shape.setFillColor(sf::Color(200, 200, 200));
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Black);

        // Center text in button
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setPosition(
            position.x + (size.x - textBounds.width) / 2,
            position.y + (size.y - textBounds.height) / 2
        );
    }

    bool contains(const sf::Vector2f& point) const {
        return shape.getGlobalBounds().contains(point);
    }

    void update(const sf::Vector2f& mousePos) {
        isHovered = contains(mousePos);
        shape.setFillColor(isHovered ? sf::Color(180, 180, 180) : sf::Color(200, 200, 200));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }
};

// Global buttons
static Button* imageButton = nullptr;
static Button* resultButton = nullptr;
static sf::Font globalFont;

// Function to initialize buttons
void initButtons(sf::RenderWindow& window) {
    if (imageButton == nullptr) {
        if (!globalFont.loadFromFile("arial.ttf")) {
            std::cerr << "Failed to load font. Please make sure arial.ttf is in the program directory.\n";
            return;
        }

        float windowWidth = window.getSize().x;
        float windowHeight = window.getSize().y;
        
        // Button Position
        sf::Vector2f basePos(windowWidth/2 - 90, windowHeight - 95);

        imageButton = new Button(
            basePos,
            sf::Vector2f(180, 40),
            "Download Image",
            globalFont
        );

        resultButton = new Button(
            sf::Vector2f(basePos.x, basePos.y + 50),
            sf::Vector2f(180, 40),
            "Download Result",
            globalFont
        );
    }
}


// Function to save routing results to a file
void saveRoutingResults() {
    std::ofstream outFile("routing_results.txt");
    if (outFile.is_open()) {
        for (const auto& [id, steps] : id_to_steps) {
            if (steps == -1)
                outFile << "Routing failed for net_id " << id << "\n";
            else
                outFile << "route id: " << id << " => steps: " << steps << "\n";
        }
        outFile.close();
    }
}

// Function to handle button clicks
bool handleButtonClick(const sf::Vector2f& mousePos, sf::RenderWindow& window, const float PanelHeightRate) {
    if (imageButton && imageButton->contains(mousePos)) {
        // full screenshot
        sf::Texture texture;
        texture.create(window.getSize().x, window.getSize().y);
        texture.update(window);
        sf::Image fullScreenshot = texture.copyToImage();

        // cropped the button area
        unsigned int croppedHeight = window.getSize().y - window.getSize().y * PanelHeightRate;
        sf::Image croppedScreenshot;
        croppedScreenshot.create(window.getSize().x, croppedHeight);

        for (unsigned int y = 0; y < croppedHeight; ++y) {
            for (unsigned int x = 0; x < window.getSize().x; ++x) {
                croppedScreenshot.setPixel(x, y, fullScreenshot.getPixel(x, y));
            }
        }

        if (croppedScreenshot.saveToFile("maze_screenshot.png")) {
            std::cout << "Screenshot saved to maze_screenshot.png\n";
        }
        return true;
    }
    else if (resultButton && resultButton->contains(mousePos)) {
        saveRoutingResults();
        std::cout << "Routing results saved to routing_results.txt\n";
        return true;
    }
    return false;
}



// Function to check if mouse is over a path cell
void updateHoverState(const Grid& g, sf::RenderWindow& window, const int cellSize) {
    // Get mouse position in world coordinates
    sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    mouse_position = sf::Mouse::getPosition(window);
    
    // Calculate grid position using world coordinates
    int col = static_cast<int>(worldPos.x / cellSize);
    int row = static_cast<int>(worldPos.y / cellSize);
    
    if (row >= 0 && row < g.M && col >= 0 && col < g.N) {
        const Cell& cell = g.grid[row][col];
        if (cell.path_id != -1 && !cell.is_start && !cell.is_end) {
            hovered_path_id = cell.path_id;
        } else {
            hovered_path_id = -1;
        }
    } else {
        hovered_path_id = -1;
    }

    // Update button hover states
    if (imageButton && resultButton) {
        imageButton->update(sf::Vector2f(mouse_position));
        resultButton->update(sf::Vector2f(mouse_position));
    }
}

// render Maze
void renderMaze(const Grid& g, sf::RenderWindow& window, const int cellSize) {
    // Initialize buttons if not already done
    initButtons(window);

    // Update hover state
    updateHoverState(g, window, cellSize);

    for (int i = 0; i < g.M; ++i) {
        for (int j = 0; j < g.N; ++j) {
            const Cell& cell = g.grid[i][j];
            sf::RectangleShape rect(sf::Vector2f(cellSize, cellSize));
            rect.setPosition(j * cellSize, i * cellSize);

            // Setting Color
            if (cell.is_obstacle) {
                rect.setFillColor(sf::Color::Black); // obstacle
            }           
            else if (cell.path_id != -1) {
                if(cell.is_start)
                    rect.setFillColor(sf::Color::Blue); // start
                else if(cell.is_end)
                    rect.setFillColor(sf::Color::Red); // end
                else if(cell.path_id == hovered_path_id)
                    rect.setFillColor(sf::Color(128, 0, 128)); // purple for hovered path
                else
                    rect.setFillColor(sf::Color(0, 255, 0)); // route
            }            
            else {
                rect.setFillColor(sf::Color(255, 255, 255)); // space
            }

            window.draw(rect);
            
            // render RoutingNumber
            if (cell.is_start || cell.is_end) {
                renderRoutingNumber(g, window, cellSize, cell.path_id);    
            }        
        }
    }

    // Render hover information
    if (hovered_path_id != -1) {
        sf::Text hoverText;
        hoverText.setFont(globalFont);
        
        // Get steps from the map, default to 0 if not found
        int steps = id_to_steps.count(hovered_path_id) ? id_to_steps[hovered_path_id] : 0;
        
        hoverText.setString("Route " + std::to_string(hovered_path_id) + 
                          "\nSteps: " + std::to_string(steps));
        hoverText.setCharacterSize(20);
        hoverText.setFillColor(sf::Color::Black);
        
        // Calculate text bounds
        sf::FloatRect textBounds = hoverText.getLocalBounds();
        float textWidth = textBounds.width + 20;  // Add padding
        float textHeight = textBounds.height + 20;  // Add padding
        
        // Get mouse position in world coordinates
        sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        
        // Fixed offset from cursor (in pixels)
        const float offsetX = 10.0f;
        const float offsetY = -40.0f;
        
        // Calculate text position
        float xPos = worldPos.x + offsetX;
        float yPos = worldPos.y + offsetY;
        
        // Get window size
        sf::Vector2u windowSize = window.getSize();
        
        // Adjust position if text would go beyond window bounds
        if (xPos + textWidth > windowSize.x) {
            xPos = worldPos.x - textWidth - offsetX;
        }
        if (yPos < 0) {
            yPos = worldPos.y + 10;
        }
        
        // Position the text
        hoverText.setPosition(xPos, yPos);
        
        // Add a background rectangle for better visibility
        sf::RectangleShape textBg;
        textBg.setSize(sf::Vector2f(textWidth, textHeight));
        textBg.setPosition(xPos - 10, yPos - 10);
        textBg.setFillColor(sf::Color(255, 255, 255, 200));
        
        window.draw(textBg);
        window.draw(hoverText);
    }

    // Draw buttons
    if (imageButton && resultButton) {
        imageButton->draw(window);
        resultButton->draw(window);
    }
}

// render RoutingNumber
void renderRoutingNumber(const Grid& g, sf::RenderWindow& window, const int cellSize, const int id) {
    sf::Text startText;
    startText.setFont(globalFont);
   
    Cell* start = g.net_points.at(id).first;
    Cell* end = g.net_points.at(id).second;

    // Starting Point Text
    startText.setString(std::to_string(id));
    startText.setCharacterSize(cellSize / 2);
    startText.setFillColor(sf::Color::White);

    // Set Text Center
    sf::FloatRect startBounds = startText.getLocalBounds();
    startText.setOrigin(startBounds.left + startBounds.width / 2.0f,
                        startBounds.top + startBounds.height / 2.0f);
    startText.setPosition(start->y * cellSize + cellSize / 2.0f,
                        start->x * cellSize + cellSize / 2.0f);
    window.draw(startText);

    // Ending Point Text
    sf::Text endText;
    endText.setFont(globalFont);
    endText.setString(std::to_string(id));
    endText.setCharacterSize(cellSize / 2);
    endText.setFillColor(sf::Color::White);

    // Set Text Center
    sf::FloatRect endBounds = endText.getLocalBounds();
    endText.setOrigin(endBounds.left + endBounds.width / 2.0f,
                    endBounds.top + endBounds.height / 2.0f);
    endText.setPosition(end->y * cellSize + cellSize / 2.0f,
                        end->x * cellSize + cellSize / 2.0f);
    window.draw(endText);   
}
