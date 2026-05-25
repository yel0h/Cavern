#include "src/core/Game.hpp"
#include <iostream>

int main()
{
    Game game;
    try
    {
        game.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}