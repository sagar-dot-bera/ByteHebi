#pragma once
#include "snake.h"
#include "fruit.h"
#include <string>

class Game
{
public:
    // Optional player name; defaults to "Player"
    Game(int width, int height, const std::string &name = "Player");
    ~Game();

    // Run the game loop (blocking). Returns final score.
    int run();

private:
    void processInput();
    void update();
    void render() const;
    void gameOverScreen() const;
    void reset();
    void chooseDifficulty();

    int width;
    int height;
    Snake snake;
    Fruit fruit;
    int score{0};
    bool over{false};
    bool exitRequested{false};
    std::string playerName{"Player"};
    int tickMs{120};
};
