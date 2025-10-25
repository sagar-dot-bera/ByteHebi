// Snake implementation
#include "snake.h"
#include <algorithm>

Snake::Snake(int startX, int startY, int initialLength)
    : dir(Direction::Right)
{
    // Head at start, extend to the left
    for (int i = 0; i < initialLength; ++i)
    {
        body.push_back({startX - i, startY});
    }
}

void Snake::setDirection(Direction d)
{
    // prevent reversing direction directly
    if ((dir == Direction::Up && d == Direction::Down) ||
        (dir == Direction::Down && d == Direction::Up) ||
        (dir == Direction::Left && d == Direction::Right) ||
        (dir == Direction::Right && d == Direction::Left))
    {
        return;
    }
    dir = d;
}

Point Snake::nextHead() const
{
    Point h = body.front();
    switch (dir)
    {
    case Direction::Up:
        --h.y;
        break;
    case Direction::Down:
        ++h.y;
        break;
    case Direction::Left:
        --h.x;
        break;
    case Direction::Right:
        ++h.x;
        break;
    }
    return h;
}

bool Snake::hitsSelf(const Point &next) const
{
    return std::any_of(body.begin(), body.end(), [&](const Point &p)
                       { return p == next; });
}

bool Snake::contains(const Point &p) const
{
    return std::any_of(body.begin(), body.end(), [&](const Point &s)
                       { return s == p; });
}

void Snake::move(bool grow)
{
    Point newHead = nextHead();
    body.push_front(newHead);
    if (!grow)
    {
        body.pop_back();
    }
}
