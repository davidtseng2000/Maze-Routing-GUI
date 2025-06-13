#ifndef DRAW_H
#define DRAW_H

#include <SFML/Graphics.hpp>
#include "objects.h"
#include <string>
#include <unordered_map>
#include <map>

using namespace std;

// Function declarations
void renderMaze(const Grid& g, sf::RenderWindow& window, const int cellSize);
void renderRoutingNumber(const Grid& g, sf::RenderWindow& window, const int cellSize, const int path_id);
bool handleButtonClick(const sf::Vector2f& mousePos, sf::RenderWindow& window, const float PanelHeightRate);

#endif
