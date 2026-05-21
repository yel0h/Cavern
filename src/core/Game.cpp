#include "Game.hpp"
#include <iostream>

Game *Game::inst = nullptr;

void Game::framebufferSizeCB(GLFWwindow *w, int width, int height)
{
    if (inst)
    {
        inst->winW = width;
        inst->winH = height;
    }
}

void Game::init()
{
    inst = this;
    if (!glfwInit())
    {
        throw std::runtime_error("glfwInit failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(winW, winH, "Cavern", nullptr, nullptr);
    if (!window)
    {
        throw std::runtime_error("glfwCreateWindow failed");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    if (!gladLoadGL())
    {
        throw std::runtime_error("gladLoadGL failed");
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCB);
    input.init(window);
    renderer.init();
    std::cout << "Generating world..." << std::endl;
    auto t0 = glfwGetTime();
    world.generate((unsigned int)std::time(nullptr));
    std::cout << "World generated in " << glfwGetTime() - t0 << " s" << std::endl;
    player.respawn();
    timer.start();
}

void Game::tick()
{
    player.tick(0.01666667, world, input);
    camera.position = player.eyePos();
    camera.yaw = player.yaw;
    camera.pitch = player.pitch;
}

void Game::render()
{
    renderer.renderFrame(world, camera, winW, winH);
    glfwSwapBuffers(window);
}

void Game::run()
{
    init();
    while (!glfwWindowShouldClose(window))
    {
        input.beginFrame();
        glfwPollEvents();
        int ticks = timer.advance();
        for (int i = 0; i < ticks; ++i)
        {
            tick();
        }

        render();
    }

    shutdown();
}

void Game::shutdown()
{
    renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}