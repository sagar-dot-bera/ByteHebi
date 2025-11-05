#include <iostream>
#include "game.h"

int main()
{
    // Grid size (including walls). Playable area is (width-2) x (height-2)
    // Increased default board: m x n where m>n
    const int width = 80;  // m (horizontal)
    const int height = 30; // n (vertical)
    // Game will prompt for player name in an in-game dialog on startup
    Game game(width, height);
    game.run();
    return 0;
}