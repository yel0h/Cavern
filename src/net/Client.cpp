#include "Client.hpp"
#include "Packet.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <ws2tcpip.h>

bool Client::connect(const std::string &host, unsigned short port)
{
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    unsigned long nb = 1;
    ioctlsocket(sock, FIONBIO, &nb);
    int r = ::connect(sock, reinterpret_cast<sockaddr const *>(&addr), sizeof(addr));
    if (r != 0 && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);
    timeval tv{3, 0};
    if (select(0, nullptr, &wfds, nullptr, &tv) != 1)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    int sockErr = 0;
    int errLen = sizeof(sockErr);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&sockErr), &errLen);
    if (sockErr != 0)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    PktJoin j{};
    std::strncpy(j.name, localName, 15);
    j.name[15] = '\0';
    send(sock, reinterpret_cast<char const *>(&j), sizeof(j), 0);
    return true;
}

void Client::disconnect()
{
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    remote.clear();
}

void Client::tick()
{
    if (sock != INVALID_SOCKET)
    {
        drainRecv();
    }
}

void Client::setLocalName(const char *n)
{
    std::strncpy(localName, n, 15);
    localName[15] = '\0';
}

void Client::sendPosition(float x, float y, float z, float yaw)
{
    if (sock == INVALID_SOCKET)
    {
        return;
    }

    float dx = x - lastSentX;
    float dy = y - lastSentY;
    float dz = z - lastSentZ;
    if ((dx * dx) + (dy * dy) + (dz * dz) < 0.0004f && std::abs(yaw - lastSentYaw) < 0.5f)
    {
        return;
    }

    lastSentX = x;
    lastSentY = y;
    lastSentZ = z;
    lastSentYaw = yaw;
    PktPos pp{};
    pp.type = (unsigned char)PktType::Pos;
    pp.id = localId;
    pp.x = x;
    pp.y = y;
    pp.z = z;
    pp.yaw = yaw;
    send(sock, reinterpret_cast<char const *>(&pp), sizeof(pp), 0);
}

void Client::sendBreak(int bx, int by, int bz, unsigned char bt) const
{
    if (sock == INVALID_SOCKET)
    {
        return;
    }

    PktBreak pk{};
    pk.bx = bx;
    pk.by = by;
    pk.bz = bz;
    pk.blockType = bt;
    send(sock, reinterpret_cast<char const *>(&pk), sizeof(pk), 0);
}

void Client::sendPlace(int bx, int by, int bz, unsigned char bt, float px, float py, float pz) const
{
    if (sock == INVALID_SOCKET)
    {
        return;
    }

    PktPlace pk{};
    pk.bx = bx;
    pk.by = by;
    pk.bz = bz;
    pk.blockType = bt;
    pk.px = px;
    pk.py = py;
    pk.pz = pz;
    send(sock, reinterpret_cast<char const *>(&pk), sizeof(pk), 0);
}

void Client::sendChat(const char *msg) const
{
    if (sock == INVALID_SOCKET)
    {
        return;
    }

    PktChat pk{};
    pk.senderId = localId;
    std::strncpy(pk.msg, msg, 127);
    pk.msg[127] = '\0';
    send(sock, reinterpret_cast<char const *>(&pk), sizeof(pk), 0);
}

void Client::interpolate(float dt)
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
        while (r.vyaw >= 360.f)
        {
            r.vyaw -= 360.f;
        }

        while (r.vyaw < 0.f)
        {
            r.vyaw += 360.f;
        }

        float moved = std::sqrt(((r.vx - prevVx) * (r.vx - prevVx)) + ((r.vz - prevVz) * (r.vz - prevVz)));
        r.walkPhase += moved * 3.f;
    }
}

void Client::drainRecv()
{
    char tmp[512];
    bool disconnected = false;
    while (true)
    {
        int n = recv(sock, tmp, sizeof(tmp), 0);
        if (n == 0)
        {
            disconnected = true;
            break;
        }

        if (n < 0)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                break;
            }

            disconnected = true;
            break;
        }

        buf.insert(buf.end(), tmp, tmp + n);
    }

    while (!buf.empty())
    {
        unsigned char t = buf[0];
        if (t == (unsigned char)PktType::Welcome)
        {
            if (buf.size() < sizeof(PktWelcome))
            {
                break;
            }

            PktWelcome wlc;
            memcpy(&wlc, buf.data(), sizeof(wlc));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktWelcome));
            localId = wlc.id;
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
            if (pp.id == localId)
            {
                continue;
            }

            bool found = false;
            for (auto &r : remote)
            {
                if (r.id == pp.id)
                {
                    r.x = pp.x;
                    r.y = pp.y;
                    r.z = pp.z;
                    r.yaw = pp.yaw;
                    if (!r.posInitialized)
                    {
                        r.vx = pp.x;
                        r.vy = pp.y;
                        r.vz = pp.z;
                        r.vyaw = pp.yaw;
                        r.posInitialized = true;
                    }

                    found = true;
                    break;
                }
            }

            if (!found)
            {
                RemotePlayer rp{};
                rp.id = pp.id;
                rp.x = pp.x;
                rp.y = pp.y;
                rp.z = pp.z;
                rp.yaw = pp.yaw;
                rp.vx = pp.x;
                rp.vy = pp.y;
                rp.vz = pp.z;
                rp.vyaw = pp.yaw;
                rp.posInitialized = true;
                remote.push_back(rp);
            }
        }
        else if (t == (unsigned char)PktType::Leave)
        {
            if (buf.size() < sizeof(PktLeave))
            {
                break;
            }

            PktLeave lv;
            memcpy(&lv, buf.data(), sizeof(lv));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktLeave));
            remote.erase(std::remove_if(remote.begin(), remote.end(),
                                          [&](const RemotePlayer& r) { return r.id == lv.id; }), remote.end());
        }
        else if (t == (unsigned char)PktType::Info)
        {
            if (buf.size() < sizeof(PktInfo))
            {
                break;
            }

            PktInfo pi;
            std::memcpy(&pi, buf.data(), sizeof(pi));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktInfo));
            bool found = false;
            for (auto &r : remote)
            {
                if (r.id == pi.id)
                {
                    std::strncpy(r.name, pi.name, 15);
                    r.name[15] = '\0';
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                RemotePlayer rp{};
                rp.id = pi.id;
                std::strncpy(rp.name, pi.name, 15);
                rp.name[15] = '\0';
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
            pk.msg[sizeof(pk.msg) - 1] = '\0';
            ChatEvent ev{};
            ev.isPrivate = pk.isPrivate;
            if (pk.isPrivate == 0)
            {
                if (localId == pk.senderId)
                {
                    std::strncpy(ev.name, localName, 16);
                }
                else
                {
                    for (const auto &r : remote)
                    {
                        if (r.id == pk.senderId)
                        {
                            std::strncpy(ev.name, r.name, 16);
                            break;
                        }
                    }
                }
            }

            std::strncpy(ev.msg, pk.msg, 128);
            pendingChats.push_back(ev);
        }
        else if (t == (unsigned char)PktType::LevelChunk)
        {
            if (buf.size() < sizeof(PktLevelChunk))
            {
                break;
            }

            PktLevelChunk pk;
            std::memcpy(&pk, buf.data(), sizeof(pk));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktLevelChunk));
            LevelChunkEvent ev{};
            ev.cx = pk.cx;
            ev.cz = pk.cz;
            std::memcpy(ev.blocks, pk.blocks, sizeof(ev.blocks));
            pendingLevelChunks.push_back(ev);
        }
        else if (t == (unsigned char)PktType::Expel)
        {
            if (buf.size() < sizeof(PktExpel))
            {
                break;
            }

            PktExpel pk;
            std::memcpy(&pk, buf.data(), sizeof(pk));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktExpel));
            ChatEvent ev{};
            std::strncpy(ev.name, "Server", 16);
            std::strncpy(ev.msg, pk.reason, 128);
            pendingChats.push_back(ev);
            disconnect();
            return;
        }
        else if (t == (unsigned char)PktType::WardenStatus)
        {
            if (buf.size() < sizeof(PktWardenStatus))
            {
                break;
            }

            PktWardenStatus pk;
            std::memcpy(&pk, buf.data(), sizeof(pk));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktWardenStatus));
            ChatEvent ev{};
            std::strncpy(ev.name, "Server", 16);
            std::strncpy(ev.msg, pk.granted
                                         ? "You have been granted warden status."
                                         : "Your warden status has been revoked.", 128);
            pendingChats.push_back(ev);
        }
        else if (t == (unsigned char)PktType::Spawn)
        {
            if (buf.size() < sizeof(PktSpawn))
            {
                break;
            }

            PktSpawn pk;
            std::memcpy(&pk, buf.data(), sizeof(pk));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktSpawn));
            pendingSpawnX = pk.x;
            pendingSpawnZ = pk.z;
            hasPendingSpawn = true;
        }
        else if (t == (unsigned char)PktType::Place)
        {
            if (buf.size() < sizeof(PktPlace))
            {
                break;
            }

            PktPlace pk;
            std::memcpy(&pk, buf.data(), sizeof(pk));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktPlace));
            pendingPlaces.push_back({pk.bx, pk.by, pk.bz, pk.blockType});
        }
        else if (t == (unsigned char)PktType::Warp)
        {
            if (buf.size() < sizeof(PktWarp))
            {
                break;
            }

            PktWarp pk;
            std::memcpy(&pk, buf.data(), sizeof(pk));
            buf.erase(buf.begin(), buf.begin() + sizeof(PktWarp));
            pendingWarpX = pk.x;
            pendingWarpY = pk.y;
            pendingWarpZ = pk.z;
            hasPendingWarp = true;
        }
        else
        {
            std::cerr << "Client: unknown packet " << (short)t << " from server, dropping connection" << std::endl;
            buf.clear();
            break;
        }
    }

    if (disconnected)
    {
        disconnect();
    }
}