// Lab 4 Part B: tests written against lab4-base with no source changes.
// Only snake.cpp and fruit.cpp can be linked; game.cpp includes notcurses.
#include <gtest/gtest.h>
#include "snake.h"
#include "fruit.h"

// Rule 1: pressing the opposite direction is ignored.
TEST(Rule1_NoReverse, OppositeDirectionIsIgnored)
{
    Snake s(10, 5, 3); // head (10,5), moving Right
    s.setDirection(Direction::Left);
    EXPECT_EQ(s.getDirection(), Direction::Right);
    s.move();
    EXPECT_EQ(s.head(), (Point{11, 5})) << "snake must keep moving right, not into its own neck";
}

TEST(Rule1_NoReverse, PerpendicularTurnIsAccepted)
{
    Snake s(10, 5, 3);
    s.setDirection(Direction::Up);
    EXPECT_EQ(s.getDirection(), Direction::Up);
}

// Rule 5: fruit never appears on a cell the snake occupies.
TEST(Rule5_FruitNotOnSnake, RespawnAvoidsSnakeBody)
{
    Snake s(40, 15, 3);
    Fruit f(80, 30);
    for (int i = 0; i < 200; ++i)
    {
        f.respawn([&](const Point &p)
                  { return s.contains(p); });
        ASSERT_FALSE(s.contains(f.position())) << "iteration " << i;
    }
}

// Board 80x3 has one playable row (y == 1); a snake of length 78 fills it.
// With no free cell, respawn falls back to {1,1}, which is on the snake.
TEST(Rule5_FruitNotOnSnake, FullBoardDoesNotPlaceFruitOnSnake)
{
    Snake s(78, 1, 78); // covers x = 1..78, y = 1
    Fruit f(80, 3);
    f.respawn([&](const Point &p)
              { return s.contains(p); });
    EXPECT_FALSE(s.contains(f.position()))
        << "fruit placed at (" << f.position().x << "," << f.position().y << ") on the snake";
}
