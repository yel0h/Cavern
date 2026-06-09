#include "Game.hpp"
#include "../net/Server.hpp"
#include "../net/Client.hpp"
#include <fstream>
#include <iostream>

Game *Game::inst = nullptr;

Game::Game() = default;

Game::~Game() = default;

void Game::setNetMode(bool host, const std::string &sJoinIp)
{
    isHost = host;
    joinIp = sJoinIp;
}

static void saveSpawnFile(float x, float z)
{
    std::ofstream f("spawn.dat", std::ios::binary);
    if (!f)
    {
        return;
    }

    f.write(reinterpret_cast<char const *>(&x), sizeof(x));
    f.write(reinterpret_cast<char const *>(&z), sizeof(z));
}

static bool loadSpawnFile(float &x, float &z)
{
    std::ifstream f("spawn.dat", std::ios::binary);
    if (!f)
    {
        return false;
    }

    bool ok = f.read(reinterpret_cast<char *>(&x), sizeof(x)) && f.read(reinterpret_cast<char *>(&z), sizeof(z));
    return ok;
}

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
    if (!wanderers.load("wanderers.dat"))
    {
        wanderers.spawn(world);
    }

    float sx, sz;
    if (loadSpawnFile(sx, sz))
    {
        player.loadSpawn(sx, sz);
    }

    player.respawn();
    timer.start();
    lastFrameTime = glfwGetTime();
    if (isHost)
    {
        server = std::make_unique<Server>();
        if (!server->start(5565))
        {
            std::cerr << "Server: failed to bind port 5565" << std::endl;
        }
        else
        {
            std::cout << "Server listening on port 5565" << std::endl;
        }
    }

    if (!joinIp.empty())
    {
        client = std::make_unique<Client>();
        if (!client->connect(joinIp, 5565))
        {
            std::cerr << "Client: failed to connect to " << joinIp << ":5565" << std::endl;
        }
        else
        {
            std::cout << "Connected to " << joinIp << ":5565" << std::endl;
        }
    }
}

static constexpr BlockType hotbar[] = {
        BlockType::Stone, BlockType::Rubble, BlockType::Soil, BlockType::Timber,
        BlockType::Boards, BlockType::Sapling, BlockType::Silt, BlockType::Grit
};

static constexpr int hotbarSize = 8;

void Game::tick()
{
    auto holdRepeat = [](bool held, bool &action, int &timer)
    {
        if (!held)
        {
            timer = 0;
            return;
        }

        if (action)
        {
            timer = 0;
            return;
        }

        if (++timer >= 15)
        {
            action = true;
            timer = 0;
        }
    };
    holdRepeat(input.primaryHeld, input.primaryAction, primaryHoldTimer);
    holdRepeat(input.switchHeld, input.switchMode, switchHoldTimer);
    player.tick(0.01666667, world, input);
    if (player.lastBroken.valid)
    {
        particles.spawnFromBlock(player.lastBroken.bx, player.lastBroken.by, player.lastBroken.bz, player.lastBroken.type);
    }

    particles.tick(0.01666667, world);
    wanderers.tick((float)Timer::dt, world);
    world.tickDynamic();
    for (int i = 0; i < hotbarSize; i++)
    {
        if (input.getSlot(i))
        {
            hotbarIdx = i;
        }
    }

    int scrollDelta = input.getScrollDelta();
    if (scrollDelta != 0)
    {
        hotbarIdx = (((hotbarIdx + scrollDelta) % hotbarSize) + hotbarSize) % hotbarSize;
    }

    player.selectedBlock = hotbar[hotbarIdx];
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
        player.setSpawn();
        saveSpawnFile(player.spawnX, player.spawnZ);
        world.save("world.dat");
        std::cout << "Spawn point set." << std::endl;
    }

    if (server)
    {
        server->setHostPos(player.position.x, player.position.y, player.position.z, player.yaw);
        server->tick();
        remotePlayers = server->remote;
    }

    if (client)
    {
        client->tick();
        client->sendPosition(player.position.x, player.position.y, player.position.z, player.yaw);
        remotePlayers = client->remote;
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
    player.resetSpawn();
    player.respawn();
    wanderers.reset();
    std::remove("wanderers.dat");
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
    if (!remotePlayers.empty())
    {
        wandererRenderer.renderRemotePlayers(remotePlayers, camera, winW, winH, (float)glfwGetTime());
    }

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
            input.setCaptureMode(!paused);
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
                input.setCaptureMode(true);
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
    if (server)
    {
        server->shutdown();
        server.reset();
    }

    if (client)
    {
        client->disconnect();
        client.reset();
    }

    world.save("world.dat");
    wanderers.save("wanderers.dat");
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