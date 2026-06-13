#include "Server.hpp"
#include "Packet.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

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
    broadcastExcept(&pk, sizeof(pk), INVALID_SOCKET);
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
    PktPos hp{};
    hp.type = (unsigned char)PktType::Pos;
    hp.id = 0;
    hp.x = host.x;
    hp.y = host.y;
    hp.z = host.z;
    hp.yaw = host.yaw;
    broadcastExcept(&hp, sizeof(hp), INVALID_SOCKET);
}

void Server::acceptClients()
{
    while (true)
    {
        SOCKET s = accept(listenSock, nullptr, nullptr);
        if (s == INVALID_SOCKET)
        {
            break;
        }

        unsigned long nb = 1;
        ioctlsocket(s, FIONBIO, &nb);
        if ((int)clients.size() >= maxClients)
        {
            closesocket(s);
            continue;
        }

        ClientState cs;
        cs.sock = s;
        cs.id = nextId++;
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
                    cs.levelSentChunks = 0;
                    std::strncpy(cs.name, jn.name, 15);
                    cs.name[15] = '\0';
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
                broadcastExcept(&pk, sizeof(pk), cs.sock);
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
                broadcastExcept(&pk, sizeof(pk), cs.sock);
                ChatEvent ev{};
                std::strncpy(ev.name, cs.name, 16);
                std::strncpy(ev.msg, pk.msg, 128);
                pendingChats.push_back(ev);
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