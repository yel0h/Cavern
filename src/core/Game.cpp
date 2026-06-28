#include "Game.hpp"
#include "../net/Client.hpp"
#include "../net/Server.hpp"
#include "src/world/Lighting.hpp"
#include <cstring>
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

void Game::setLocalName(const std::string &n)
{
    std::strncpy(localName, n.c_str(), 15);
    localName[15] = '\0';
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

        server->world = &world;
        server->setHostName(localName);
    }

    if (!joinIp.empty())
    {
        client = std::make_unique<Client>();
        client->setLocalName(localName);
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
        BlockType::Stone, BlockType::Soil, BlockType::Pith, BlockType::Boards,
        BlockType::Sapling, BlockType::Timber, BlockType::Glaze, BlockType::Grit
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

        if (++timer >= 4)
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
        auto bt = (unsigned char)player.lastBroken.type;
        int bx = player.lastBroken.bx;
        int by = player.lastBroken.by;
        int bz = player.lastBroken.bz;
        if (client)
        {
            client->sendBreak(bx, by, bz, bt);
        }

        if (server)
        {
            server->broadcastBreak(bx, by, bz, bt);
        }
    }

    if (player.lastPlaced.valid)
    {
        auto bt = (unsigned char)player.lastPlaced.type;
        int bx = player.lastPlaced.bx;
        int by = player.lastPlaced.by;
        int bz = player.lastPlaced.bz;
        float px = player.position.x;
        float py = player.position.y;
        float pz = player.position.z;
        if (client)
        {
            client->sendPlace(bx, by, bz, bt, px, py, pz);
        }

        if (server)
        {
            server->broadcastPlace(bx, by, bz, bt);
        }
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
    if (input.getSpawnMob() && !client)
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
        server->interpolate(0.01666667);
        remotePlayers = server->remote;
    }

    if (client)
    {
        client->tick();
        if (client->hasPendingSpawn)
        {
            player.loadSpawn(client->pendingSpawnX, client->pendingSpawnZ);
            player.respawn();
            client->clearPendingSpawn();
        }

        if (client->hasPendingWarp)
        {
            player.position.x = client->pendingWarpX;
            player.position.y = client->pendingWarpY;
            player.position.z = client->pendingWarpZ;
            player.velocity = {};
            client->clearPendingWarp();
        }

        client->sendPosition(player.position.x, player.position.y, player.position.z, player.yaw);
        client->interpolate(0.01666667);
        remotePlayers = client->remote;
    }

    auto spawnRemoteBreak = [&](const BreakEvent &b)
    {
        particles.spawnFromBlock(b.bx, b.by, b.bz, (BlockType)b.blockType);
    };
    if (client)
    {
        for (const auto &b : client->pendingBreaks)
        {
            spawnRemoteBreak(b);
        }

        client->clearBreaks();
    }

    if (server)
    {
        for (const auto &b : server->pendingBreaks)
        {
            spawnRemoteBreak(b);
        }

        server->clearBreaks();
    }

    auto applyRemotePlace = [&](const PlaceEvent &p)
    {
        if (World::inBounds(p.bx, p.by, p.bz))
        {
            world.setBlock(p.bx, p.by, p.bz, (BlockType)p.blockType);
            Lighting::propagateColumn(world, p.bx, p.bz);
        }
    };
    if (client)
    {
        for (const auto &p : client->pendingPlaces)
        {
            applyRemotePlace(p);
        }

        client->clearPlaces();
    }

    if (server)
    {
        for (const auto &p : server->pendingPlaces)
        {
            applyRemotePlace(p);
        }

        server->clearPlaces();
    }

    if (client && !client->pendingLevelChunks.empty())
    {
        for (const auto &e : client->pendingLevelChunks)
        {
            if (Chunk *ch = world.getChunk(e.cx, e.cz))
            {
                std::memcpy(ch->blocks.data(), e.blocks, sizeof(e.blocks));
                ch->dirty = true;
            }
        }

        Lighting::propagate(world);
        client->clearLevelChunks();
    }

    auto addChat = [&](const ChatEvent &e)
    {
        std::string line = (e.isPrivate == 1) ? std::string(e.msg) : std::string("[") + e.name + "]: " + e.msg;
        chatMessages.push_back(std::move(line));
        if (chatMessages.size() > 20)
        {
            chatMessages.erase(chatMessages.begin());
        }
    };
    if (client)
    {
        for (const auto &e : client->pendingChats)
        {
            addChat(e);
        }

        client->clearChats();
    }

    if (server)
    {
        for (const auto &e : server->pendingChats)
        {
            addChat(e);
        }

        server->clearChats();
    }
}

void Game::generateNewLevel()
{
    if (server || client)
    {
        return;
    }

    renderer.renderGenerating(winW, winH);
    glfwSwapBuffers(window);
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
                         hotbarSize, hotbarIdx,
                         fps, chunks, player.placeMode,
                         player.underLava, player.underWater, hotbarOpen);
    wandererRenderer.render(wanderers, camera, winW, winH, (float)glfwGetTime());
    if (!remotePlayers.empty())
    {
        wandererRenderer.renderRemotePlayers(remotePlayers, camera, winW, winH);
        renderer.renderPlayerNames(remotePlayers, camera, winW, winH);
    }

    float aspect = (winH > 0) ? (float)winW / (float)winH : 1.f;
    glm::mat4 vp = camera.viewProjection(aspect);
    particleRenderer.render(particles.particles, vp);
    if (paused)
    {
        renderer.renderPauseMenu(winW, winH);
    }

    renderer.renderChat(chatMessages, input.chatOpen, input.chatBuffer, winW, winH);
    if (input.tabHeld && (server || client))
    {
        std::vector<std::string> playerNames;
        playerNames.emplace_back(localName[0] ? localName : "Player");
        for (const auto &r : remotePlayers)
        {
            playerNames.emplace_back(r.name);
        }

        renderer.renderPlayerList(playerNames, winW, winH);
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
            if (input.getHotbarToggle())
            {
                hotbarOpen = !hotbarOpen;
            }

            if (input.getChatToggle() && (server || client))
            {
                input.chatOpen = !input.chatOpen;
                input.chatBuffer.clear();
                input.setCaptureMode(!input.chatOpen);
            }

            if (input.getChatSubmit() && !input.chatBuffer.empty())
            {
                bool isCmd = input.chatBuffer[0] == '/';
                if (!isCmd)
                {
                    std::string name(localName[0] ? localName : "Player");
                    std::string line = "[" + name + "]: " + input.chatBuffer;
                    chatMessages.push_back(std::move(line));
                    if (chatMessages.size() > 20)
                    {
                        chatMessages.erase(chatMessages.begin());
                    }
                }

                if (client)
                {
                    client->sendChat(input.chatBuffer.c_str());
                }

                if (server && !client)
                {
                    if (isCmd)
                    {
                        server->handleHostCommand(input.chatBuffer.c_str());
                    }
                    else
                    {
                        server->broadcastChat(0, input.chatBuffer.c_str());
                    }
                }

                input.chatBuffer.clear();
                input.chatOpen = false;
                input.setCaptureMode(true);
            }

            if (input.getChatCancel() && input.chatOpen)
            {
                input.chatOpen = false;
                input.chatBuffer.clear();
                input.setCaptureMode(true);
            }

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
        static constexpr double frameBudget = 1.0 / 100.0;
        double deadline = lastFrameTime + frameBudget;
        while (glfwGetTime() < deadline) {}
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