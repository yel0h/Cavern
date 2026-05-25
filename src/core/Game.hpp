#ifndef CAVERN_GAME_HPP
#define CAVERN_GAME_HPP
#include "Input.hpp"
#include "Timer.hpp"
#include "src/entity/ParticleManager.hpp"
#include "src/entity/WandererManager.hpp"
#include "src/player/Player.hpp"
#include "src/render/ParticleRenderer.hpp"
#include "src/render/Renderer.hpp"
#include "src/render/WandererRenderer.hpp"
#include "src/world/World.hpp"
#include <GLFW/glfw3.h>

class Game
{
private:
    GLFWwindow* window = nullptr;
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
    int winW = 640;
    int winH = 480;
    bool glfwInitialized = false;
    bool isFullscreen = false;
    int storedX = 0;
    int storedY = 0;
    int storedW = 640;
    int storedH = 480;
    int fps = 0;
    int frameCount = 0;
    int chunks = 0;
    double fpsTimer = 0.0;
    double lastFrameTime = 0.0;
    static Game *inst;

    void init();

    void tick();

    void render();

    void shutdown();

    void toggleFullscreen();

    static void framebufferSizeCB(GLFWwindow *w, int width, int height);

public:
    void run();
};
#endif//CAVERN_GAME_HPP