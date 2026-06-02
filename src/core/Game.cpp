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

    glfwInitialized = true;
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
    seed = (unsigned int)std::time(nullptr);
    if (!world.load("world.dat"))
    {
        std::cout << "Generating world..." << std::endl;
        auto t0 = glfwGetTime();
        world.generate(seed);
        std::cout << "World generated in " << glfwGetTime() - t0 << " s" << std::endl;
    }
    else
    {
        std::cout << "World loaded from world.dat" << std::endl;
    }

    wandererRenderer.init();
    particleRenderer.init();
    player.respawn();
    timer.start();
    lastFrameTime = glfwGetTime();
}

void Game::tick()
{
    player.tick(0.01666667, world, input);
    if (player.lastBroken.valid)
    {
        particles.spawnFromBlock(player.lastBroken.bx, player.lastBroken.by, player.lastBroken.bz, player.lastBroken.type);
    }

    particles.tick(0.01666667, world);
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

    if (input.getSlot(4))
    {
        player.selectedBlock = BlockType::Boards;
    }

    if (input.getSlot(5))
    {
        player.selectedBlock = BlockType::Sapling;
    }

    if (input.getSpawnMob())
    {
        wanderers.spawnOne(world, player.position.x, player.position.z);
    }

    if (input.getCycleFog())
    {
        renderer.cycleFog();
    }

    if (input.getNewLevel())
    {
        generateNewLevel();
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
}

void Game::generateNewLevel()
{
    renderer.renderGenerating(winW, winH);
    glfwSwapBuffers(window);
    seed++;
    for (int cz = 0; cz < World::CHUNKS_Z; cz++)
    {
        for (int cx = 0; cx < World::CHUNKS_X; cx++)
        {
            Chunk *c = world.getChunk(cx, cz);
            if (!c)
            {
                continue;
            }

            c->blocks.fill(BlockType::Air);
            c->dirty = true;
        }
    }

    std::cout << "Generating new world (seed "<< seed <<")..." << std::endl;
    auto t0 = glfwGetTime();
    world.generate(seed);
    std::cout << "World generated in " << glfwGetTime() - t0 << " s" << std::endl;
    renderer.markAllDirty();
    player.respawn();
    wanderers.reset();
}

void Game::render()
{
    Renderer::HighlightBlock hl;
    if (player.hitBlock.valid)
    {
        hl = {true, player.hitBlock.bx, player.hitBlock.by, player.hitBlock.bz};
    }

    renderer.renderFrame(world, camera, winW, winH,
                           hl, (float)glfwGetTime(),
                           player.selectedBlock,
                           fps, chunks, player.placeMode,
                           player.underLava);
    wandererRenderer.render(wanderers, camera, winW, winH, (float)glfwGetTime());
    float aspect = (winH > 0) ? (float)winW / (float)winH : 1.f;
    glm::mat4 vp = camera.viewProjection(aspect);
    particleRenderer.render(particles.particles, vp);
    if (paused)
    {
        renderer.renderPauseMenu(winW, winH);
    }

    glfwSwapBuffers(window);
}

void Game::run()
{
    try
    {
        init();
    }
    catch (...)
    {
        shutdown();
        throw;
    }

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        fpsTimer += now - lastFrameTime;
        lastFrameTime = now;
        frameCount++;
        if (fpsTimer >= 1.0)
        {
            fps = frameCount;
            frameCount = 0;
            chunks = renderer.lastChunkUpdates;
            renderer.lastChunkUpdates = 0;
            fpsTimer -= 1.0;
        }

        input.beginFrame();
        glfwPollEvents();
        if (input.getPauseToggle())
        {
            paused = !paused;
            glfwSetInputMode(window, GLFW_CURSOR, paused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            input.resetMouseState();
        }

        if (paused)
        {
            for (int i = 0; i < 5; i++)
            {
                char path[32];
                if (input.getFuncKey(i))
                {
                    std::snprintf(path, sizeof(path), "save_%d.dat", i);
                    world.save(path);
                    std::cout << "Saved to slot " << i + 1 << '.' << std::endl;
                }

                if (input.getFuncKey(i + 5))
                {
                    std::snprintf(path, sizeof(path), "save_%d.dat", i);
                    if (world.load(path))
                    {
                        renderer.markAllDirty();
                        player.respawn();
                        std::cout << "Loaded slot " << i + 1 << '.' << std::endl;
                    }
                }
            }

            if (input.getNewLevel())
            {
                generateNewLevel();
                paused = false;
            }
        }
        else
        {
            player.applyMouseLook(input);
            camera.position = player.eyePos();
            camera.yaw = player.yaw;
            camera.pitch = player.pitch;
            int ticks = timer.advance();
            for (int i = 0; i < ticks; ++i)
            {
                tick();
            }
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
    particleRenderer.shutdown();
    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if (glfwInitialized)
    {
        glfwTerminate();
        glfwInitialized = false;
    }

    inst = nullptr;
}