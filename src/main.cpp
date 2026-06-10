#define WIN32_LEAN_AND_MEAN
#include "src/core/Game.hpp"
#include <iostream>
#include <winsock.h>

int main(int argc, char *argv[])
{
    bool doHost = false;
    std::string joinIp;
    std::string playerName;
    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "--host")
        {
            doHost = true;
        }
        else if (std::string(argv[i]) == "--join" && i + 1 < argc)
        {
            joinIp = argv[++i];
        }
        else if (std::string(argv[i]) == "--name" && i + 1 < argc)
        {
            playerName = argv[++i];
        }
    }

    if (playerName.empty())
    {
        playerName = "Player";
    }

    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    Game game;
    game.setNetMode(doHost, joinIp);
    game.setLocalName(playerName);
    try
    {
        game.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << std::endl;
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}