#pragma once
#include <deque>
#include "point.h"

enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

class Snake
{
public:
    Snake(int startX, int startY, int initialLength = 3);

    // Advance one step. If grow is true, don't remove tail.
    void move(bool grow = false);
    // Change direction if it's not directly opposite
    void setDirection(Direction d);
    Direction getDirection() const { return dir; }

    const Point &head() const { return body.front(); }
    const std::deque<Point> &segments() const { return body; }

    bool hitsSelf(const Point &nextHead) const;
    bool contains(const Point &p) const;

    // Compute next head position given current direction
    Point nextHead() const;

private:
    std::deque<Point> body;
    Direction dir;
};
