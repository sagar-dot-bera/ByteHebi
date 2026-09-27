// Game rules, kept free of Notcurses so they can be built and tested alone
#include "rules.h"

TickResult advance(Snake &snake, Point fruit, int width, int height,
                   int &score, int &highScore,
                   const std::function<void(const Point &next)> &respawnFruit)
{
    // Compute next head and collisions
    Point next = snake.nextHead();

    // Walls
    if (next.x <= 0 || next.x >= width - 1 || next.y <= 0 || next.y >= height - 1)
        return TickResult::Died;

    // Self
    if (snake.hitsSelf(next))
        return TickResult::Died;

    bool grow = false;
    if (next == fruit)
    {
        grow = true;
        score += 10;
        if (score > highScore)
            highScore = score;
        respawnFruit(next);
    }

    snake.move(grow);
    return grow ? TickResult::Ate : TickResult::Moved;
}
