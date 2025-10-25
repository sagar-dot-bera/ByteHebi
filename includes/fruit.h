#pragma once
#include "point.h"
#include <random>

class Fruit
{
public:
    Fruit(int width, int height);

    const Point &position() const { return pos; }
    // Respawn fruit at a random free position not overlapping the snake
    template <typename ContainsFn>
    void respawn(ContainsFn &&isOccupied);

private:
    int width;
    int height;
    Point pos{0, 0};
    std::mt19937 rng;
};

template <typename ContainsFn>
void Fruit::respawn(ContainsFn &&isOccupied)
{
    std::uniform_int_distribution<int> dx(1, width - 2);  // inside borders
    std::uniform_int_distribution<int> dy(1, height - 2); // inside borders
    for (int tries = 0; tries < 1000; ++tries)
    {
        Point candidate{dx(rng), dy(rng)};
        if (!isOccupied(candidate))
        {
            pos = candidate;
            return;
        }
    }
    // Fallback: place at 1,1 if somehow blocked
    pos = {1, 1};
}
