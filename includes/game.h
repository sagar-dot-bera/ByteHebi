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
    enum class SnakeGlyphStyle
    {
        Light,
        Heavy,
        Rounded,
        Scales,
        DoubleLine,
        Block,
        Arrow,
        Dotted,
        Braille
    };
    enum class DialogType
    {
        None,
        Pause,
        GameOver,
        EnterName
    };
    void processInput();
    void update();
    void render() const;

    void reset();
    void chooseDifficulty();
    void loadHighScore();
    void saveHighScore();
    void openDialog(DialogType t);
    void closeDialog();

    int width;
    int height;
    Snake snake;
    Fruit fruit;
    int score{0};
    bool over{false};
    bool exitRequested{false};
    bool paused{false};
    bool dialogOpen{false};
    DialogType dialogType{DialogType::None};
    int dialogIndex{0};
    std::string playerName{"Player"};
    // Temporary storage for name entry when EnterName dialog is open
    std::string nameEntry{""};
    int nameMaxLen{24};
    int tickMs{120};
    int highScore{0};
    std::string highScoreFile{"highscore.txt"};
    SnakeGlyphStyle snakeStyle{SnakeGlyphStyle::Heavy};
};
