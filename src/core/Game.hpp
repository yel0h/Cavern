#ifndef CAVERN_GAME_HPP
#define CAVERN_GAME_HPP
#include "Input.hpp"
#include "Timer.hpp"
#include "src/audio/Audio.hpp"
#include "src/entity/ParticleManager.hpp"
#include "src/entity/WandererManager.hpp"
#include "src/net/NetTypes.hpp"
#include "src/player/Player.hpp"
#include "src/render/ParticleRenderer.hpp"
#include "src/render/Renderer.hpp"
#include "src/render/WandererRenderer.hpp"
#include "src/world/World.hpp"
#include <GLFW/glfw3.h>

class Server;
class Client;

class Game
{
private:
    GLFWwindow* window = nullptr;
    Audio audio;
    Input input;
    Timer timer;
    World world;
    Renderer renderer;
    Camera camera;
    Player player;
    WandererManager wanderers;
    WandererRenderer wandererRenderer;
    ParticleManager particles;
    ParticleRenderer particleRenderer;
    int winW = 854;
    int winH = 480;
    bool glfwInitialized = false;
    bool isFullscreen = false;
    bool paused = false;
    int storedX = 0;
    int storedY = 0;
    int storedW = 854;
    int storedH = 480;
    int fps = 0;
    int frameCount = 0;
    int chunks = 0;
    double fpsTimer = 0.0;
    double lastFrameTime = 0.0;
    unsigned int seed = 0;
    int hotbarIdx = 0;
    int primaryHoldTimer = 0;
    int switchHoldTimer = 0;
    bool inventoryOpen = false;
    bool isHost = false;
    std::string joinIp;
    char localName[16] = {};
    std::unique_ptr<Server> server;
    std::unique_ptr<Client> client;
    std::vector<RemotePlayer> remotePlayers;
    std::vector<std::string> chatMessages;
    bool hotbarOpen = true;
    static Game *inst;

    void init();

    void tick();

    void render();

    void shutdown();

    void toggleFullscreen();

    void generateNewLevel();

    static void framebufferSizeCB(GLFWwindow *w, int width, int height);

public:
    Game();

    ~Game();

    void run();

    void setNetMode(bool host, const std::string &sJoinIp);

    void setLocalName(const std::string &n);
};
#endif//CAVERN_GAME_HPP