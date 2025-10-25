#include <iostream>
#include "game.h"

int main()
{
    // Grid size (including walls). Playable area is (width-2) x (height-2)
    const int width = 40;
    const int height = 20;
    Game game(width, height);
    game.run();
    return 0;
}