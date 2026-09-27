// Lab 4 Part D: Rule 4, made testable by the advance() seam in rules.h.
#include <gtest/gtest.h>
#include <vector>
#include "rules.h"

// Spy: records every cell the rules asked the game to respawn the fruit from.
struct RespawnSpy
{
    std::vector<Point> calls;
    std::function<void(const Point &)> fn()
    {
        return [this](const Point &next)
        { calls.push_back(next); };
    }
};

// Rule 4: eating a fruit adds exactly 10 to the score and grows the snake by one.
TEST(Rule4_EatFruit, EatingAddsExactlyTenToScore)
{
    Snake snake(10, 5, 3); // head (10,5), moving Right
    int score = 30, highScore = 100;
    RespawnSpy spy;

    TickResult r = advance(snake, Point{11, 5}, 20, 10, score, highScore, spy.fn());

    EXPECT_EQ(r, TickResult::Ate);
    EXPECT_EQ(score, 40);
}

TEST(Rule4_EatFruit, EatingGrowsSnakeByOneWithHeadOnFruit)
{
    Snake snake(10, 5, 3);
    int score = 0, highScore = 0;
    RespawnSpy spy;

    advance(snake, Point{11, 5}, 20, 10, score, highScore, spy.fn());

    EXPECT_EQ(snake.segments().size(), 4u);
    EXPECT_EQ(snake.head(), (Point{11, 5}));
}

TEST(Rule4_EatFruit, EatingAsksForExactlyOneRespawnAtTheEatenCell)
{
    Snake snake(10, 5, 3);
    int score = 0, highScore = 0;
    RespawnSpy spy;

    advance(snake, Point{11, 5}, 20, 10, score, highScore, spy.fn());

    ASSERT_EQ(spy.calls.size(), 1u);
    EXPECT_EQ(spy.calls[0], (Point{11, 5})) << "new fruit must also avoid the cell the head moves into";
}

TEST(Rule4_EatFruit, MissingTheFruitLeavesScoreLengthAndFruitAlone)
{
    Snake snake(10, 5, 3);
    int score = 30, highScore = 100;
    RespawnSpy spy;

    TickResult r = advance(snake, Point{15, 7}, 20, 10, score, highScore, spy.fn());

    EXPECT_EQ(r, TickResult::Moved);
    EXPECT_EQ(score, 30);
    EXPECT_EQ(snake.segments().size(), 3u);
    EXPECT_TRUE(spy.calls.empty());
}

TEST(Rule4_EatFruit, EatingPastTheHighScoreRaisesIt)
{
    Snake snake(10, 5, 3);
    int score = 100, highScore = 100;
    RespawnSpy spy;

    advance(snake, Point{11, 5}, 20, 10, score, highScore, spy.fn());

    EXPECT_EQ(highScore, 110);
}
