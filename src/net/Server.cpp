#include "Server.hpp"
#include "Packet.hpp"
#include <algorithm>

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

void Server::tick()
{
    acceptClients();
    drainClients();
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
        while (true)
        {
            int n = recv(cs.sock, tmp, sizeof(tmp), 0);
            if (n == 0)
            {
                removeClient(i);
                break;
            }

            if (n < 0)
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                {
                    break;
                }

                removeClient(i);
                break;
            }

            cs.buf.insert(cs.buf.end(), tmp, tmp + n);
        }
        if (i >= (int)clients.size())
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

                buf.erase(buf.begin(), buf.begin() + sizeof(PktJoin));
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
                    remote.push_back({cs.id, pp.x, pp.y, pp.z, pp.yaw});
                }
            }
            else
            {
                buf.clear();
                break;
            }
        }
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