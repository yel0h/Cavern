#include "Game.hpp"
#include "../net/Client.hpp"
#include "../net/Server.hpp"
#include "src/world/Lighting.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

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

static constexpr BlockType hotbar[] = {
        BlockType::Pith, BlockType::Stone, BlockType::Soil, BlockType::Boards,
        BlockType::Sapling, BlockType::Timber, BlockType::Glaze, BlockType::Grit,
        BlockType::GoldBlock,
        BlockType::WeavePale, BlockType::WeaveAsh, BlockType::WeaveSlate,
        BlockType::WeaveRust, BlockType::WeaveBurn, BlockType::WeaveGlow,
        BlockType::WeaveBlight, BlockType::WeaveMold, BlockType::WeaveFern,
        BlockType::WeaveFrost, BlockType::WeaveAzure,BlockType::WeaveDeep,
        BlockType::WeaveDusk, BlockType::WeaveMurk, BlockType::WeaveBloom,
        BlockType::WeaveBlush,
        BlockType::Goldenbloom, BlockType::Thornbloom,
        BlockType::Dustshroom, BlockType::Emberscap,
};
static constexpr int hotbarSize = 29;

static int hotbarIndexFor(BlockType t)
{
    for (int i = 0; i < hotbarSize; i++)
    {
        if (hotbar[i] == t)
        {
            return i;
        }
    }

    return -1;
}

static bool blockDropFor(BlockType t, BlockType &outType, int &outCount)
{
    static std::mt19937 mt{std::random_device()()};
    static std::uniform_int_distribution<int> timberDist(3, 5);
    static std::uniform_int_distribution<int> saplingDist(0, 9);
    switch (t)
    {
        case BlockType::Air:
        case BlockType::Bedrock:
        case BlockType::Lava:
        case BlockType::LavaStill:
        case BlockType::Water:
        case BlockType::WaterStill:
            return false;

        case BlockType::Turf:
            outType = BlockType::Soil;
            outCount = 1;
            return true;

        case BlockType::Timber:
            outType = BlockType::Boards;
            outCount = timberDist(mt);
            return true;

        case BlockType::Sapling:
            if (saplingDist(mt) != 0)
            {
                return false;
            }

            outType = BlockType::Sapling;
            outCount = 1;
            return true;

        default:
            outType = t;
            outCount = 1;
            return true;
    }
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
    settings.load();
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
    audio.init();
    audio.setSfxEnabled(settings.soundEnabled);
    audio.setMusicEnabled(settings.musicEnabled);
    input.init(window);
    input.bindForward = settings.keyForward;
    input.bindBackward = settings.keyBackward;
    input.bindLeft = settings.keyLeft;
    input.bindRight = settings.keyRight;
    input.bindJump = settings.keyJump;
    input.bindSave = settings.keySave;
    input.bindCycleFog = settings.keyCycleFog;
    input.bindNewLevel = settings.keyNewLevel;
    input.bindFullscreen = settings.keyFullscreen;
    input.bindChat = settings.keyChat;
    input.bindInventory = settings.keyInventory;
    input.bindThrowBolt = settings.keyThrowBolt;
    input.bindPlaceSign = settings.keyPlaceSign;
    input.invertY = settings.invertMouse;
    renderer.init();
    renderer.setFogLevel(settings.renderDistance);
    renderer.showFps = settings.showFps;
    renderer.buildFullIconAtlas(hotbar, hotbarSize);
    hotbarCounts.assign(hotbarSize, 0);
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
    mobRenderer.init();
    if (!wanderers.load("wanderers.dat"))
    {
        wanderers.spawn(world);
    }

    if (!mobs.load("mobs.dat"))
    {
        mobs.spawn(world);
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
        player.isWarden = false;
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
    if (inventoryOpen)
    {
        input.primaryAction = false;
    }

    if (input.primaryAction && !player.placeMode && !inventoryOpen && !player.isDown)
    {
        constexpr int fistDamage = 2;
        constexpr float attackReach = 4.5f;
        glm::vec3 eye = player.eyePos();
        float yr = glm::radians(player.yaw);
        float pr = glm::radians(player.pitch);
        glm::vec3 forward{std::cos(pr) * std::sin(yr), std::sin(pr), -std::cos(pr) * std::cos(yr)};
        bool killed = false;
        MobType killedType{};
        if (mobs.attack(eye, forward, attackReach, fistDamage, killed, killedType))
        {
            input.primaryAction = false;
            if (killed)
            {
                switch (killedType)
                {
                    case MobType::Snout:
                        score += 15;
                        break;

                    case MobType::Boneshade:
                    case MobType::Grubbin:
                        score += 75;
                        break;

                    case MobType::Fumewretch:
                        score += 180;
                        break;
                }
            }
        }
    }

    player.tick(0.01666667, world, input);
    if (player.isDown && screen == ScreenState::World)
    {
        screen = ScreenState::Dead;
        input.setCaptureMode(false);
    }

    if ((player.justLanded || player.footstepReady) && player.blockBelow != BlockType::Air)
    {
        audio.queueFootstep(Audio::blockSound(player.blockBelow));
    }

    if (player.lastBroken.valid)
    {
        audio.queueBreak(Audio::blockSound(player.lastBroken.type));
        particles.spawnFromBlock(player.lastBroken.bx, player.lastBroken.by, player.lastBroken.bz, player.lastBroken.type);
        BlockType dropType;
        int dropCount;
        if (blockDropFor(player.lastBroken.type, dropType, dropCount))
        {
            glm::vec3 dropPos{player.lastBroken.bx + 0.5f, player.lastBroken.by + 0.3f, player.lastBroken.bz + 0.5f};
            itemDrops.spawn(dropPos, dropType, dropCount);
        }

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
        if (server && server->forgeMode && player.lastPlaced.type == BlockType::Stone)
        {
            int bx = player.lastPlaced.bx;
            int by = player.lastPlaced.by;
            int bz = player.lastPlaced.bz;
            world.setBlock(bx, by, bz, BlockType::Bedrock);
            Lighting::propagateColumn(world, bx, bz);
            player.lastPlaced.type = BlockType::Bedrock;
        }

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
    mobs.tick((float)Timer::dt, world, player, particles);
    world.tickDynamic();
    if (input.getThrowBolt() && !inventoryOpen && !player.isDown)
    {
        float yr = glm::radians(player.yaw);
        float pr = glm::radians(player.pitch);
        glm::vec3 dir{std::cos(pr) * std::sin(yr), std::sin(pr), -std::cos(pr) * std::cos(yr)};
        projectiles.spawn(player.eyePos(), dir);
    }

    std::vector<MobType> kills;
    projectiles.tick(0.01666667, world, mobs, kills);
    for (MobType t : kills)
    {
        switch (t)
        {
            case MobType::Snout:
                score += 15;
                break;

            case MobType::Boneshade:
            case MobType::Grubbin:
                score += 75;
                break;

            case MobType::Fumewretch:
                score += 180;
                break;
        }
    }

    itemDrops.tick(0.01666667, world, player.position);
    for (const PickupEvent &ev : itemDrops.drainPickups())
    {
        int idx = hotbarIndexFor(ev.type);
        if (idx >= 0)
        {
            hotbarCounts[idx] += ev.count;
        }
    }

    for (int i = 0; i < 8; i++)
    {
        if (input.getSlot(i))
        {
            hotbarIdx = i;
        }
    }

    int scrollDelta = input.getScrollDelta();
    if (scrollDelta != 0)
    {
        hotbarIdx = (((hotbarIdx + scrollDelta) % 8) + 8) % 8;
    }

    player.selectedBlock = hotbar[hotbarIdx];
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

    auto applyRemoteBreak = [&](const BreakEvent &b)
    {
        particles.spawnFromBlock(b.bx, b.by, b.bz, (BlockType)b.blockType);
        if (World::inBounds(b.bx, b.by, b.bz))
        {
            world.setBlock(b.bx, b.by, b.bz, BlockType::Air);
            Lighting::propagateColumn(world, b.bx, b.bz);
        }
    };
    if (client)
    {
        for (const auto &b : client->pendingBreaks)
        {
            applyRemoteBreak(b);
        }

        client->clearBreaks();
    }

    if (server)
    {
        for (const auto &b : server->pendingBreaks)
        {
            applyRemoteBreak(b);
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

    audio.flushSounds();
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
    mobs.reset();
    mobs.spawn(world);
    std::remove("mobs.dat");
}

void Game::restartLevel()
{
    player.respawn();
    mobs.reset();
    mobs.spawn(world);
    std::remove("mobs.dat");
    score = 0;
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
                         8, hotbarIdx,
                         fps, chunks, player.placeMode,
                         player.underLava, player.underWater, hotbarOpen,
                         player.vitality, Player::maxVitality);
    wandererRenderer.render(wanderers, camera, winW, winH, (float)glfwGetTime());
    mobRenderer.render(mobs, camera, winW, winH, (float)glfwGetTime());
    if (!remotePlayers.empty())
    {
        wandererRenderer.renderRemotePlayers(remotePlayers, camera, winW, winH);
        renderer.renderPlayerNames(remotePlayers, camera, winW, winH);
    }

    float aspect = (winH > 0) ? (float)winW / (float)winH : 1.f;
    glm::mat4 vp = camera.viewProjection(aspect);
    particleRenderer.render(particles.particles, vp);
    particleRenderer.renderDrops(itemDrops.drops, vp);
    particleRenderer.renderBolts(projectiles.bolts, vp);
    if (inventoryOpen)
    {
        renderer.renderInventory(winW, winH, player.selectedBlock, input.mouseX, input.mouseY, hotbar, hotbarSize);
    }

    if (screen == ScreenState::Paused)
    {
        renderer.renderPauseMenu(winW, winH, input.mouseX, input.mouseY);
    }
    else if (screen == ScreenState::Options)
    {
        renderer.renderOptionsMenu(winW, winH, input.mouseX, input.mouseY, input.captureNextKey ? pendingBindAction : -1, settings);
    }
    else if (screen == ScreenState::Dead)
    {
        renderer.renderDeathScreen(winW, winH, input.mouseX, input.mouseY, score);
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

        renderer.renderPlayerList(playerNames, winW, winH, input.mouseX, input.mouseY, input.chatOpen);
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
        input.menuActive = (screen != ScreenState::World);
        glfwPollEvents();
        if (input.getPauseToggle() && screen != ScreenState::Dead)
        {
            if (inventoryOpen)
            {
                inventoryOpen = false;
                input.inventoryOpen = false;
                input.setCaptureMode(true);
            }
            else if (screen == ScreenState::Options)
            {
                screen = ScreenState::Paused;
            }
            else if (screen == ScreenState::Paused)
            {
                screen = ScreenState::World;
                input.setCaptureMode(true);
            }
            else
            {
                screen = ScreenState::Paused;
                input.setCaptureMode(false);
            }
        }

        if (screen == ScreenState::Paused)
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
                screen = ScreenState::World;
                input.setCaptureMode(true);
            }

            if (input.getMenuClick())
            {
                constexpr float btnW = 200.f, btnH = 32.f;
                constexpr int s = 3;
                float lineH = (Font::CHAR_H * s) + 6.f;
                float startY = (float)winH * 0.35f;
                float bx0 = ((float)winW * 0.5f) - (btnW * 0.5f);
                float bx1 = bx0 + btnW;
                float by0 = startY + (lineH * 6.5f);
                float by1 = by0 + btnH;
                if (input.mouseX >= bx0 && input.mouseX < bx1 && input.mouseY >= by0 && input.mouseY < by1)
                {
                    screen = ScreenState::Options;
                }
            }
        }
        else if (screen == ScreenState::Options)
        {
            if (input.capturedKey != -1)
            {
                if (input.capturedKey != -2 && pendingBindAction >= 0)
                {
                    int key = input.capturedKey;
                    switch (pendingBindAction)
                    {
                        case 0:
                            settings.keyForward = key;
                            input.bindForward = key;
                            break;

                        case 1:
                            settings.keyBackward = key;
                            input.bindBackward = key;
                            break;

                        case 2:
                            settings.keyLeft = key;
                            input.bindLeft = key;
                            break;

                        case 3:
                            settings.keyRight = key;
                            input.bindRight = key;
                            break;

                        case 4:
                            settings.keyJump = key;
                            input.bindJump = key;
                            break;

                        case 5:
                            settings.keySave = key;
                            input.bindSave = key;
                            break;

                        case 6:
                            settings.keyCycleFog = key;
                            input.bindCycleFog = key;
                            break;

                        case 7:
                            settings.keyNewLevel = key;
                            input.bindNewLevel = key;
                            break;

                        case 8:
                            settings.keyFullscreen = key;
                            input.bindFullscreen = key;
                            break;

                        case 9:
                            settings.keyChat = key;
                            input.bindChat = key;
                            break;

                        case 10:
                            settings.keyInventory = key;
                            input.bindInventory = key;
                            break;

                        case 11:
                            settings.keyThrowBolt = key;
                            input.bindThrowBolt = key;
                            break;

                        case 12:
                            settings.keyPlaceSign = key;
                            input.bindPlaceSign = key;
                            break;
                    }

                    settings.save();
                }

                input.capturedKey = -1;
                pendingBindAction = -1;
            }

            if (input.getMenuClick() && !input.captureNextKey)
            {
                constexpr float startY = 30.f;
                constexpr float rowH = 24.f;
                constexpr float rowGap = 6.f;
                constexpr float step = rowH + rowGap;
                constexpr float colW = 300.f;
                constexpr float colGap = 20.f;
                float colLeftX = ((float)winW * 0.5f) - (colGap * 0.5f) - colW;
                float colRightX = ((float)winW * 0.5f) + (colGap * 0.5f);
                float mx = input.mouseX;
                float my = input.mouseY;
                if (mx >= colLeftX && mx < colLeftX + colW)
                {
                    int row = (int)((my - startY) / step);
                    if (row >= 0 && row < 13 && my < startY + (row * step) + rowH)
                    {
                        input.captureNextKey = true;
                        pendingBindAction = row;
                    }
                }
                else if (mx >= colRightX && mx < colRightX + colW)
                {
                    int row = (int)((my - startY) / step);
                    if (row >= 0 && row < 7 && my < startY + (row * step) + rowH)
                    {
                        switch (row)
                        {
                            case 0:
                                renderer.cycleFog();
                                settings.renderDistance = renderer.fogLevel;
                                settings.save();
                                break;

                            case 1:
                                settings.invertMouse = !settings.invertMouse;
                                input.invertY = settings.invertMouse;
                                settings.save();
                                break;

                            case 2:
                                settings.soundEnabled = !settings.soundEnabled;
                                audio.setSfxEnabled(settings.soundEnabled);
                                settings.save();
                                break;

                            case 3:
                                settings.musicEnabled = !settings.musicEnabled;
                                audio.setMusicEnabled(settings.musicEnabled);
                                settings.save();
                                break;

                            case 4:
                                settings.showFps = !settings.showFps;
                                renderer.showFps = settings.showFps;
                                settings.save();
                                break;

                            case 5:
                                settings.viewBobbing = !settings.viewBobbing;
                                settings.save();
                                break;

                            case 6:
                                screen = ScreenState::Paused;

                            default:
                                break;
                        }
                    }
                }
            }
        }
        else if (screen == ScreenState::Dead)
        {
            if (input.getMenuClick())
            {
                restartLevel();
                screen = ScreenState::World;
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

            if (input.chatOpen && input.getTabListClick())
            {
                std::vector<std::string> playerNames;
                playerNames.emplace_back(localName[0] ? localName : "Player");
                for (const auto &r : remotePlayers)
                {
                    playerNames.emplace_back(r.name);
                }

                constexpr int s = 2;
                constexpr float lineH = (Font::CHAR_H * s) + 4.f;
                float rowY = 20.f + lineH;
                for (const auto &n : playerNames)
                {
                    auto nw = (float)(n.size() * Font::CHAR_W * s);
                    float rx0 = (((float)winW - nw) * 0.5f) - 4.f;
                    float rx1 = rx0 + nw + 8.f;
                    if (input.mouseX >= rx0 && input.mouseX < rx1 && input.mouseY >= rowY && input.mouseY < rowY + lineH)
                    {
                        if (input.chatBuffer.size() + n.size() + 1 < 127)
                        {
                            input.chatBuffer += n + " ";
                        }

                        break;
                    }

                    rowY += lineH;
                }
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

            if (input.getInventoryToggle() && !input.chatOpen)
            {
                inventoryOpen = !inventoryOpen;
                input.inventoryOpen = inventoryOpen;
                input.setCaptureMode(!inventoryOpen);
            }

            if (inventoryOpen && input.getInventoryClick())
            {
                static constexpr int invCols = 6;
                static constexpr float invSlot = 48.f;
                static constexpr float invGap = 4.f;
                int rows = (hotbarSize + invCols - 1) / invCols;
                float totalW = (invCols * invSlot) + ((invCols - 1) * invGap);
                float totalH = (rows * invSlot) + ((rows - 1) * invGap);
                float ox = ((float)winW - totalW) * 0.5f;
                float oy = ((float)winH - totalH) * 0.5f;
                float mx = input.mouseX;
                float my = input.mouseY;
                int col = (int)((mx - ox) / (invSlot + invGap));
                int row = (int)((my - oy) / (invSlot + invGap));
                int idx = (row * invCols) + col;
                if (col >= 0 && col < invCols && idx >= 0 && idx < hotbarSize)
                {
                    float sx = ox + (col * (invSlot + invGap));
                    float sy = oy + (row * (invSlot + invGap));
                    if (mx < sx + invSlot && my < sy + invSlot)
                    {
                        hotbarIdx = idx;
                        player.selectedBlock = hotbar[hotbarIdx];
                        inventoryOpen = false;
                        input.inventoryOpen = false;
                        input.setCaptureMode(true);
                    }
                }
            }

            player.applyMouseLook(input);
            float yr = glm::radians(player.yaw);
            float pr = glm::radians(player.pitch);
            glm::vec3 fwd{std::cos(pr) * std::sin(yr), std::sin(pr), -std::cos(pr) * std::cos(yr)};
            glm::vec3 eye = player.eyePos();
            if (settings.viewBobbing)
            {
                float hspd = std::sqrt((player.velocity.x * player.velocity.x) + (player.velocity.z * player.velocity.z));
                if (player.onGround && hspd > 0.5f)
                {
                    bobPhase += hspd * 0.045f;
                }

                float bobAmt = std::min(hspd / 6.f, 1.f);
                eye.y += std::sin(bobPhase) * 0.05f * bobAmt;
            }

            constexpr float kPivotForward = 0.06f;
            eye += fwd * kPivotForward;
            camera.position = eye;
            camera.yaw = player.yaw;
            camera.pitch = player.pitch;
            int ticks = timer.advance();
            for (int i = 0; i < ticks; ++i)
            {
                tick();
            }
        }

        audio.tickMusic();
        render();
        static constexpr double frameBudget = 1.0 / 200.0;
        double deadline = lastFrameTime + frameBudget;
        double remaining = deadline - glfwGetTime();
        if (remaining > 0.002)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(remaining - 0.001));
        }

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
    mobs.save("mobs.dat");
    audio.shutdown();
    renderer.shutdown();
    wandererRenderer.shutdown();
    particleRenderer.shutdown();
    mobRenderer.shutdown();
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