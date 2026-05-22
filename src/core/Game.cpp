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

void Game::toggleFullscreen()
{
    if (!isFullscreen)
    {
        glfwGetWindowPos(window, &storedX, &storedY);
        glfwGetWindowSize(window, &storedW, &storedH);
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        isFullscreen = true;
    }
    else
    {
        glfwSetWindowMonitor(window, nullptr, storedX, storedY, storedW, storedH, 0);
        isFullscreen = false;
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
    if (!world.load("world.dat"))
    {
        std::cout << "Generating world..." << std::endl;
        auto t0 = glfwGetTime();
        world.generate(0u);
        std::cout << "World generated in " << glfwGetTime() - t0 << " s" << std::endl;
    }
    else
    {
        std::cout << "World loaded from world.dat" << std::endl;
    }

    wandererRenderer.init();
    wanderers.spawn(world);
    player.respawn();
    timer.start();
}

void Game::tick()
{
    player.tick(0.01666667, world, input);
    wanderers.tick((float)Timer::dt, world);
    world.tickDynamic();
    if (input.getSlot(0))
    {
        player.selectedBlock = BlockType::Stone;
    }

    if (input.getSlot(1))
    {
        player.selectedBlock = BlockType::Rubble;
    }

    if (input.getSlot(2))
    {
        player.selectedBlock = BlockType::Soil;
    }

    if (input.getSlot(3))
    {
        player.selectedBlock = BlockType::Timber;
    }

    if (input.getSpawnMob())
    {
        wanderers.spawnOne(world, player.position.x, player.position.z);
    }

    if (input.getToggleFullscreen())
    {
        toggleFullscreen();
    }

    if (input.getSave())
    {
        world.save("world.dat");
        std::cout << "World saved." << std::endl;
    }

    camera.position = player.eyePos();
    camera.yaw = player.yaw;
    camera.pitch = player.pitch;
}

void Game::render()
{
    Renderer::HighlightFace hl;
    if (player.hitBlock.valid)
    {
        hl = {true, player.hitBlock.bx, player.hitBlock.by, player.hitBlock.bz, player.hitBlock.face};
    }

    renderer.renderFrame(world, camera, winW, winH, hl, (float)glfwGetTime(), player.selectedBlock);
    wandererRenderer.render(wanderers, camera, winW, winH, (float)glfwGetTime());
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
    world.save("world.dat");
    renderer.shutdown();
    wandererRenderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}