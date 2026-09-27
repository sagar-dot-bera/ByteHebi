#pragma once
#include "snake.h"
#include "point.h"
#include <functional>

enum class TickResult
{
    Moved,
    Ate,
    Died
};

// One game tick: wall/self collision, eating, scoring, and moving the snake.
// No terminal, clock, or file access. When the fruit is eaten, respawnFruit is
// called with the cell the head is about to enter, before the snake moves.
TickResult advance(Snake &snake, Point fruit, int width, int height,
                   int &score, int &highScore,
                   const std::function<void(const Point &next)> &respawnFruit);
