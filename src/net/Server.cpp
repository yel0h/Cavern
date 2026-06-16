#include "Server.hpp"
#include "Packet.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ws2tcpip.h>

bool Server::start(unsigned short port)
{
    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET)
    {
        return false;
    }

    unsigned long nb = 1;
    ioctlsocket(listenSock, FIONBIO, &nb);
    BOOL opt = TRUE;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const *>(&opt), sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(listenSock, reinterpret_cast<sockaddr const *>(&addr), sizeof(addr)) != 0)
    {
        return false;
    }

    if (listen(listenSock, SOMAXCONN) != 0)
    {
        return false;
    }

    host.id = 0;
    loadWardens();
    loadExiles();
    return true;
}

void Server::shutdown()
{
    for (auto &c : clients)
    {
        closesocket(c.sock);
    }

    clients.clear();
    if (listenSock != INVALID_SOCKET)
    {
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
    }
}

void Server::setHostPos(float x, float y, float z, float yaw)
{
    host.x = x;
    host.y = y;
    host.z = z;
    host.yaw = yaw;
}

void Server::setHostName(const char *n)
{
    std::strncpy(hostName, n, 15);
    hostName[15] = '\0';
    std::strncpy(host.name, hostName, 16);
}

void Server::broadcastBreak(int bx, int by, int bz, unsigned char bt)
{
    PktBreak pk{};
    pk.bx = bx;
    pk.by = by;
    pk.bz = bz;
    pk.blockType = bt;
    broadcastBreakExcept(pk, INVALID_SOCKET);
}

void Server::broadcastChat(unsigned int senderId, const char *msg)
{
    PktChat pk{};
    pk.senderId = senderId;
    std::strncpy(pk.msg, msg, 127);
    pk.msg[127] = '\0';
    broadcastExcept(&pk, sizeof(pk), INVALID_SOCKET);
}

void Server::tick()
{
    acceptClients();
    drainClients();
    sendPendingLevels();
    float dx = host.x - lastSentX;
    float dy = host.y - lastSentY;
    float dz = host.z - lastSentZ;
    float dyaw = host.yaw - lastSentYaw;
    if ((dx * dx) + (dy * dy) + (dz * dz) >= 0.0004f || std::abs(dyaw) >= 0.5f)
    {
        lastSentX = host.x;
        lastSentY = host.y;
        lastSentZ = host.z;
        lastSentYaw = host.yaw;
        PktPos hp{};
        hp.type = (uint8_t)PktType::Pos;
        hp.id = 0;
        hp.x = host.x;
        hp.y = host.y;
        hp.z = host.z;
        hp.yaw = host.yaw;
        broadcastExcept(&hp, sizeof(hp), INVALID_SOCKET);
    }
}

void Server::acceptClients()
{
    while (true)
    {
        sockaddr_in peer{};
        int peerLen = sizeof(peer);
        SOCKET s = accept(listenSock, reinterpret_cast<sockaddr *>(&peer), &peerLen);
        if (s == INVALID_SOCKET)
        {
            break;
        }

        unsigned long nb = 1;
        ioctlsocket(s, FIONBIO, &nb);
        char ipBuf[16] = {};
        inet_ntop(AF_INET, &peer.sin_addr, ipBuf, sizeof(ipBuf));
        for (const auto &banned : exiledIps)
        {
            if (banned == ipBuf)
            {
                closesocket(s);
                s = INVALID_SOCKET;
                break;
            }
        }

        if (s == INVALID_SOCKET)
        {
            continue;
        }

        if ((int)clients.size() >= maxClients)
        {
            closesocket(s);
            continue;
        }

        ClientState cs;
        cs.sock = s;
        cs.id = nextId++;
        std::strncpy(cs.ip, ipBuf, 15);
        cs.ip[15] = '\0';
        PktWelcome wlc{};
        wlc.type = (unsigned char)PktType::Welcome;
        wlc.id = cs.id;
        send(s, reinterpret_cast<char const *>(&wlc), sizeof(wlc), 0);
        clients.push_back(std::move(cs));
    }
}

void Server::drainClients()
{
    for (int i = (int)clients.size() - 1; i >= 0; i--)
    {
        ClientState &cs = clients[i];
        char tmp[256];
        bool removed = false;
        while (true)
        {
            int n = recv(cs.sock, tmp, sizeof(tmp), 0);
            if (n == 0)
            {
                removeClient(i);
                removed = true;
                break;
            }

            if (n < 0)
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                {
                    break;
                }

                removeClient(i);
                removed = true;
                break;
            }

            cs.buf.insert(cs.buf.end(), tmp, tmp + n);
        }

        if (removed)
        {
            continue;
        }

        auto &buf = cs.buf;
        while (!buf.empty())
        {
            unsigned char t = buf[0];
            if (t == (unsigned char)PktType::Join)
            {
                if (buf.size() < sizeof(PktJoin))
                {
                    break;
                }

                PktJoin jn;
                std::memcpy(&jn, buf.data(), sizeof(jn));
                buf.erase(buf.begin(), buf.begin() + sizeof(PktJoin));
                if (!cs.nameReceived)
                {
                    cs.nameReceived = true;
                    std::strncpy(cs.name, jn.name, 15);
                    cs.name[15] = '\0';
                    bool exiled = false;
                    for (const auto &ex : exiledNames)
                    {
                        if (ex == cs.name)
                        {
                            exiled = true;
                            break;
                        }
                    }

                    if (exiled)
                    {
                        PktExpel ep{};
                        std::strncpy(ep.reason, "You have been exiled from this server.", sizeof(ep.reason) - 1);
                        send(cs.sock, reinterpret_cast<char const *>(&ep), sizeof(ep), 0);
                        removeClient(i);
                        break;
                    }

                    cs.levelSentChunks = 0;
                    PktInfo hi{};
                    hi.id = 0;
                    std::strncpy(hi.name, hostName, 16);
                    send(cs.sock, reinterpret_cast<char const *>(&hi), sizeof(hi), 0);
                    for (auto &client : clients)
                    {
                        if (client.sock == cs.sock)
                        {
                            continue;
                        }

                        if (!client.nameReceived)
                        {
                            continue;
                        }

                        PktInfo pi{};
                        pi.id = client.id;
                        std::strncpy(pi.name, client.name, 16);
                        send(cs.sock, reinterpret_cast<char const *>(&pi), sizeof(pi), 0);
                    }

                    PktInfo ni{};
                    ni.id = cs.id;
                    std::strncpy(ni.name, cs.name, 16);
                    broadcastExcept(&ni, sizeof(ni), cs.sock);
                }
            }
            else if (t == (unsigned char)PktType::Pos)
            {
                if (buf.size() < sizeof(PktPos))
                {
                    break;
                }

                PktPos pp;
                memcpy(&pp, buf.data(), sizeof(pp));
                buf.erase(buf.begin(), buf.begin() + sizeof(PktPos));
                pp.id = cs.id;
                broadcastExcept(&pp, sizeof(pp), cs.sock);
                bool found = false;
                for (auto &r : remote)
                {
                    if (r.id == cs.id)
                    {
                        r.x = pp.x;
                        r.y = pp.y;
                        r.z = pp.z;
                        r.yaw = pp.yaw;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    RemotePlayer rp{};
                    rp.id = cs.id;
                    rp.x = pp.x;
                    rp.y = pp.y;
                    rp.z = pp.z;
                    rp.yaw = pp.yaw;
                    rp.vx = pp.x;
                    rp.vy = pp.y;
                    rp.vz = pp.z;
                    rp.vyaw = pp.yaw;
                    std::strncpy(rp.name, cs.name, 16);
                    remote.push_back(rp);
                }
            }
            else if (t == (unsigned char)PktType::Break)
            {
                if (buf.size() < sizeof(PktBreak))
                {
                    break;
                }

                PktBreak pk;
                std::memcpy(&pk, buf.data(), sizeof(pk));
                buf.erase(buf.begin(), buf.begin() + sizeof(PktBreak));
                broadcastBreakExcept(pk, cs.sock);
                pendingBreaks.push_back({pk.bx, pk.by, pk.bz, pk.blockType});
            }
            else if (t == (unsigned char)PktType::Chat)
            {
                if (buf.size() < sizeof(PktChat))
                {
                    break;
                }

                PktChat pk;
                std::memcpy(&pk, buf.data(), sizeof(pk));
                buf.erase(buf.begin(), buf.begin() + sizeof(PktChat));
                pk.senderId = cs.id;
                if (pk.msg[0] == '/')
                {
                    handleCommand(cs, pk.msg);
                }
                else
                {
                    broadcastExcept(&pk, sizeof(pk), cs.sock);
                    ChatEvent ev{};
                    ev.senderId = cs.id;
                    std::strncpy(ev.name, cs.name, 16);
                    std::strncpy(ev.msg, pk.msg, 128);
                    pendingChats.push_back(ev);
                }
            }
            else
            {
                std::cerr << "Server: unknown packet " << (short)t << " from client " << cs.id << ", dropping connection" << std::endl;
                buf.clear();
                break;
            }
        }
    }
}

void Server::sendPendingLevels()
{
    if (!world)
    {
        return;
    }

    for (auto &cs : clients)
    {
        if (cs.levelSentChunks < 0 || cs.levelSentChunks >= 256)
        {
            continue;
        }

        for (int n = 0; n < levelChunksPerTick && cs.levelSentChunks < 256; n++, cs.levelSentChunks++)
        {
            int cx = cs.levelSentChunks % World::CHUNKS_X;
            int cz = cs.levelSentChunks / World::CHUNKS_X;
            const Chunk *ch = world->getChunk(cx, cz);
            if (!ch)
            {
                continue;
            }

            PktLevelChunk pk{};
            pk.cx = (unsigned char)cx;
            pk.cz = (unsigned char)cz;
            std::memcpy(pk.blocks, ch->blocks.data(), sizeof(pk.blocks));
            send(cs.sock, reinterpret_cast<char const *>(&pk), sizeof(pk), 0);
        }

        if (cs.levelSentChunks == 256 && !cs.pendingBreakQueue.empty())
        {
            for (const auto &bpk : cs.pendingBreakQueue)
            {
                send(cs.sock, reinterpret_cast<char const *>(&bpk), sizeof(bpk), 0);
            }

            cs.pendingBreakQueue.clear();
        }
    }
}

void Server::interpolate(float dt)
{
    constexpr float rate = 10.f;
    float factor = 1.f - std::exp(-rate * dt);
    for (auto &r : remote)
    {
        float prevVx = r.vx;
        float prevVz = r.vz;
        r.vx += (r.x - r.vx) * factor;
        r.vy += (r.y - r.vy) * factor;
        r.vz += (r.z - r.vz) * factor;
        float dyaw = r.yaw - r.vyaw;
        while (dyaw > 180.f)
        {
            dyaw -= 360.f;
        }

        while (dyaw < -180.f)
        {
            dyaw += 360.f;
        }

        r.vyaw += dyaw * factor;
        float moved = std::sqrt(((r.vx - prevVx) * (r.vx - prevVx)) + ((r.vz - prevVz) * (r.vz - prevVz)));
        r.walkPhase += moved * 3.f;
    }
}

void Server::broadcastExcept(const void *data, int len, SOCKET skip)
{
    for (auto &c : clients)
    {
        if (c.sock != skip)
        {
            send(c.sock, (const char *)data, len, 0);
        }
    }
}

void Server::broadcastBreakExcept(const PktBreak &pk, SOCKET skip)
{
    for (auto &c : clients)
    {
        if (c.sock == skip)
        {
            continue;
        }

        if (c.levelSentChunks >= 0 && c.levelSentChunks < 256)
        {
            c.pendingBreakQueue.push_back(pk);
        }
        else
        {
            send(c.sock, reinterpret_cast<char const *>(&pk), sizeof(pk), 0);
        }
    }
}

void Server::removeClient(int idx)
{
    unsigned int id = clients[idx].id;
    closesocket(clients[idx].sock);
    clients.erase(clients.begin() + idx);
    PktLeave lv{};
    lv.type = (unsigned char)PktType::Leave;
    lv.id = id;
    broadcastExcept(&lv, sizeof(lv), INVALID_SOCKET);
    remote.erase(std::remove_if(remote.begin(), remote.end(),
                                  [id](const RemotePlayer &r) { return r.id == id; }), remote.end());
}

void Server::sendServerChat(SOCKET sock, const char *msg)
{
    PktChat pk{};
    pk.senderId = 0;
    std::strncpy(pk.msg, msg, 127);
    pk.msg[127] = '\0';
    send(sock, reinterpret_cast<char const *>(&pk), sizeof(pk), 0);
}

bool Server::isWarden(unsigned int id, const char *name) const
{
    if (id == 0)
    {
        return true;
    }

    return std::ranges::any_of(wardens, [name](const auto &w)
                               {
                                   return w == name;
                               });
}

std::vector<std::string> Server::loadTextList(const char *path)
{
    std::vector<std::string> list;
    std::ifstream f(path);
    if (!f.is_open())
    {
        return list;
    }

    std::string line;
    while (std::getline(f, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        if (line.size() > 15)
        {
            line.resize(15);
        }

        list.push_back(line);
    }

    return list;
}

void Server::saveTextList(const char *path, const std::vector<std::string> &list, const char *header)
{
    std::ofstream f(path);
    if (!f.is_open())
    {
        return;
    }

    f << header << "\n";
    for (const auto &entry : list)
    {
        f << entry << "\n";
    }
}

void Server::loadExiles()
{
    exiledNames = loadTextList("exile_list.txt");
    exiledIps = loadTextList("exiled_ips.txt");
}

void Server::saveExiles() const
{
    saveTextList("exile_list.txt", exiledNames, "# Cavern exile list");
    saveTextList("exiled_ips.txt", exiledIps, "# Cavern exiled IPs - remove an entry here to pardon an IP");
}

void Server::handleHostCommand(const char *raw)
{
    ClientState cHost{};
    cHost.id = 0;
    std::strncpy(host.name, hostName, 15);
    cHost.name[15] = '\0';
    cHost.sock = INVALID_SOCKET;
    handleCommand(cHost, raw);
}

void Server::handleCommand(Server::ClientState &sender, const char *raw)
{
    if (!isWarden(sender.id, sender.name))
    {
        if (sender.sock != INVALID_SOCKET)
        {
            sendServerChat(sender.sock, "[Server]: You do not have warden status.");
        }

        return;
    }

    char cmd[64] = {};
    const char *arg = "";
    const char *sp = std::strchr(raw + 1, ' ');
    if (sp)
    {
        auto cmdLen = (unsigned long long)(sp - raw);
        if (cmdLen >= sizeof(cmd))
        {
            cmdLen = sizeof(cmd) - 1;
        }

        std::strncpy(cmd, raw, cmdLen);
        cmd[cmdLen] = '\0';
        arg = sp + 1;
    }
    else
    {
        std::strncpy(cmd, raw, sizeof(cmd) - 1);
    }

    auto findClient = [&](const char *name) -> int
    {
        for (int i = 0; i < (int)clients.size(); i++)
        {
            if (clients[i].nameReceived && std::strcmp(clients[i].name, name) == 0)
            {
                return i;
            }
        }

        return -1;
    };

    auto replyTo = [&](const char *msg)
    {
        if (sender.sock != INVALID_SOCKET)
        {
            sendServerChat(sender.sock, msg);
        }
        else
        {
            std::cerr << msg << std::endl;
        }
    };

    if (_stricmp(cmd, "/warden") == 0)
    {
        if (!*arg)
        {
            replyTo("[Server]: Usage: /warden <name>");
            return;
        }

        int idx = findClient(arg);
        if (idx < 0)
        {
            replyTo("[Server]: Player not found.");
            return;
        }

        if (clients[idx].id == 0)
        {
            replyTo("[Server]: The host cannot be targeted.");
            return;
        }

        const char *name = clients[idx].name;
        if (!std::any_of(wardens.begin(), wardens.end(), [name](const std::string &w) { return w == name; }))
        {
            wardens.emplace_back(name);
        }

        saveWardens();
        PktWardenStatus ws{};
        ws.granted = 1;
        send(clients[idx].sock, reinterpret_cast<char const *>(&ws), sizeof(ws), 0);
        replyTo("[Server]: Warden status granted.");
    }
    else if (_stricmp(cmd, "/unwarden") == 0)
    {
        if (!*arg)
        {
            replyTo("[Server]: Usage: /unwarden <name>");
            return;
        }

        int idx = findClient(arg);
        if (idx < 0)
        {
            replyTo("[Server]: Player not found.");
            return;
        }

        if (clients[idx].id == 0)
        {
            replyTo("[Server]: The host cannot be targeted.");
            return;
        }

        const char *name = clients[idx].name;
        wardens.erase(std::remove(wardens.begin(), wardens.end(), std::string(name)), wardens.end());
        saveWardens();
        PktWardenStatus ws{};
        ws.granted = 0;
        send(clients[idx].sock, reinterpret_cast<char const *>(&ws), sizeof(ws), 0);
        replyTo("[Server]: Warden status revoked.");
    }
    else if (_stricmp(cmd, "/exile") == 0)
    {
        if (!*arg)
        {
            replyTo("[Server]: Usage: /exile <name>");
            return;
        }

        if (!std::any_of(exiledNames.begin(), exiledNames.end(), [arg](const std::string &e) { return e == arg; }))
        {
            exiledNames.emplace_back(arg);
        }

        saveExiles();
        int idx = findClient(arg);
        if (idx >= 0)
        {
            if (clients[idx].id == 0)
            {
                replyTo("[Server]: The host cannot be targeted.");
                return;
            }

            PktExpel ep{};
            std::strncpy(ep.reason, "You have been exiled from this server.", sizeof(ep.reason) - 1);
            send(clients[idx].sock, reinterpret_cast<char const *>(&ep), sizeof(ep), 0);
            removeClient(idx);
        }

        replyTo("[Server]: Player exiled.");
    }
    else if (_stricmp(cmd, "/pardon") == 0)
    {
        if (!*arg)
        {
            replyTo("[Server]: Usage: /pardon <name>");
            return;
        }

        exiledNames.erase(std::remove(exiledNames.begin(), exiledNames.end(), std::string(arg)), exiledNames.end());
        saveExiles();
        replyTo("[Server]: Player pardoned.");
    }
    else if (_stricmp(cmd, "/exileip") == 0)
    {
        if (!*arg)
        {
            replyTo("[Server]: Usage: /exileip <ip>");
            return;
        }

        if (!std::any_of(exiledIps.begin(), exiledIps.end(), [arg](const std::string &e) { return e == arg; }))
        {
            exiledIps.emplace_back(arg);
        }

        saveExiles();
        std::cerr << "[Server] IP " << arg << " exiled. Edit exiled_ips.txt to pardon." << std::endl;
        for (int i = (int)clients.size() - 1; i >= 0; i--)
        {
            if (std::strcmp(clients[i].ip, arg) == 0)
            {
                if (clients[i].id == 0)
                {
                    continue;
                }

                PktExpel ep{};
                std::strncpy(ep.reason, "Your IP has been exiled from this server.", sizeof(ep.reason) - 1);
                send(clients[i].sock, reinterpret_cast<char const *>(&ep), sizeof(ep), 0);
                removeClient(i);
            }
        }

        replyTo("[Server]: IP exiled.");
    }
    else if (_stricmp(cmd, "/expel") == 0)
    {
        if (!*arg)
        {
            replyTo("[Server]: Usage: /expel <name>");
            return;
        }

        int idx = findClient(arg);
        if (idx < 0)
        {
            replyTo("[Server]: Player not found.");
            return;
        }

        if (clients[idx].id == 0)
        {
            replyTo("[Server]: The host cannot be targeted.");
            return;
        }

        PktExpel ep{};
        std::strncpy(ep.reason, "You have been expelled from the cavern.", sizeof(ep.reason) - 1);
        send(clients[idx].sock, reinterpret_cast<char const *>(&ep), sizeof(ep), 0);
        removeClient(idx);
        replyTo("[Server]: Player expelled.");
    }
    else if (_stricmp(cmd, "/announce") == 0)
    {
        if (!*arg)
        {
            replyTo("[Server]: Usage: /announce <message>");
            return;
        }

        broadcastChat(0, arg);
        ChatEvent ev{};
        ev.senderId = 0;
        std::strncpy(ev.name, "Server", 16);
        std::strncpy(ev.msg, arg, 128);
        pendingChats.push_back(ev);
    }
    else
    {
        replyTo("[Server]: Unknown command.");
    }
}