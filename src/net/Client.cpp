#include "Client.hpp"
#include "Packet.hpp"
#include <algorithm>
#include <cmath>
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

    if (::connect(sock, reinterpret_cast<sockaddr const *>(&addr), sizeof(addr)) != 0)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    unsigned long nb = 1;
    ioctlsocket(sock, FIONBIO, &nb);
    PktJoin j{};
    j.type = (unsigned char)PktType::Join;
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

void Client::sendPosition(float x, float y, float z, float yaw) const
{
    if (sock == INVALID_SOCKET)
    {
        return;
    }

    PktPos pp{};
    pp.type = (unsigned char)PktType::Pos;
    pp.id = localId;
    pp.x = x;
    pp.y = y;
    pp.z = z;
    pp.yaw = yaw;
    send(sock, reinterpret_cast<char const *>(&pp), sizeof(pp), 0);
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
        float moved = std::sqrt(((r.vx - prevVx) * (r.vx - prevVx)) + ((r.vz - prevVz) * (r.vz - prevVz)));
        r.walkPhase += moved * 3.f;
    }
}

void Client::drainRecv()
{
    char tmp[512];
    while (true)
    {
        int n = recv(sock, tmp, sizeof(tmp), 0);
        if (n == 0)
        {
            disconnect();
            return;
        }

        if (n < 0)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                break;
            }

            disconnect();
            return;
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
        else
        {
            buf.clear();
            break;
        }
    }
}