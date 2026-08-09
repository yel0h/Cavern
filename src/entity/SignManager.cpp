#include "SignManager.hpp"
#include <fstream>

static constexpr unsigned int signMagic = 0x53494743u;

void SignManager::place(glm::vec3 pos, float yaw)
{
    if ((int)signs.size() >= maxSigns)
    {
        return;
    }

    signs.push_back({pos, yaw});
}

bool SignManager::save(const char *path) const
{
    std::ofstream f(path, std::ios::binary);
    if (!f)
    {
        return false;
    }

    f.write(reinterpret_cast<char const *>(&signMagic), sizeof(signMagic));
    unsigned int count = (unsigned int)signs.size();
    f.write(reinterpret_cast<char const *>(&count), sizeof(count));
    for (const auto &s : signs)
    {
        f.write(reinterpret_cast<char const *>(&s.position.x), sizeof(float));
        f.write(reinterpret_cast<char const *>(&s.position.y), sizeof(float));
        f.write(reinterpret_cast<char const *>(&s.position.z), sizeof(float));
        f.write(reinterpret_cast<char const *>(&s.yaw), sizeof(float));
    }

    return true;
}

bool SignManager::load(const char *path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        return false;
    }

    unsigned int magic = 0;
    f.read(reinterpret_cast<char *>(&magic), sizeof(magic));
    if (magic != signMagic)
    {
        return false;
    }

    unsigned int count = 0;
    f.read(reinterpret_cast<char *>(&count), sizeof(count));
    if (count > (unsigned int)maxSigns)
    {
        return false;
    }

    signs.clear();
    for (unsigned int i = 0; i < count; i++)
    {
        Sign s{};
        f.read(reinterpret_cast<char *>(&s.position.x), sizeof(float));
        f.read(reinterpret_cast<char *>(&s.position.y), sizeof(float));
        f.read(reinterpret_cast<char *>(&s.position.z), sizeof(float));
        f.read(reinterpret_cast<char *>(&s.yaw), sizeof(float));
        signs.push_back(s);
    }

    return true;
}