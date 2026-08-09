#include "Settings.hpp"
#include <fstream>
#include <string>

static void trim(std::string &s)
{
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

bool Settings::load(const char *path)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(f, line))
    {
        unsigned long long eq = line.find('=');
        if (eq == std::string::npos)
        {
            continue;
        }

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trim(key);
        trim(val);
        if (key.empty() || val.empty())
        {
            continue;
        }

        int ival = std::atoi(val.c_str());
        bool bval = ival != 0;
        if (key == "keyForward")
        {
            keyForward = ival;
        }
        else if (key == "keyBackward")
        {
            keyBackward = ival;
        }
        else if (key == "keyLeft")
        {
            keyLeft = ival;
        }
        else if (key == "keyRight")
        {
            keyRight = ival;
        }
        else if (key == "keyJump")
        {
            keyJump = ival;
        }
        else if (key == "keySave")
        {
            keySave = ival;
        }
        else if (key == "keyCycleFog")
        {
            keyCycleFog = ival;
        }
        else if (key == "keyNewLevel")
        {
            keyNewLevel = ival;
        }
        else if (key == "keyFullscreen")
        {
            keyFullscreen = ival;
        }
        else if (key == "keyChat")
        {
            keyChat = ival;
        }
        else if (key == "keyInventory")
        {
            keyInventory = ival;
        }
        else if (key == "keyThrowBolt")
        {
            keyThrowBolt = ival;
        }
        else if (key == "keyPlaceSign")
        {
            keyPlaceSign = ival;
        }
        else if (key == "renderDistance")
        {
            renderDistance = ival;
        }
        else if (key == "worldSizeIdx")
        {
            worldSizeIdx = ival;
        }
        else if (key == "invertMouse")
        {
            invertMouse = bval;
        }
        else if (key == "soundEnabled")
        {
            soundEnabled = bval;
        }
        else if (key == "musicEnabled")
        {
            musicEnabled = bval;
        }
        else if (key == "showFps")
        {
            showFps = bval;
        }
        else if (key == "viewBobbing")
        {
            viewBobbing = bval;
        }
    }

    return true;
}

bool Settings::save(const char *path) const
{
    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open())
    {
        return false;
    }

    f << "keyForward=" << keyForward << '\n';
    f << "keyBackward=" << keyBackward << '\n';
    f << "keyLeft=" << keyLeft << '\n';
    f << "keyRight=" << keyRight << '\n';
    f << "keyJump=" << keyJump << '\n';
    f << "keySave=" << keySave << '\n';
    f << "keyCycleFog=" << keyCycleFog << '\n';
    f << "keyNewLevel=" << keyNewLevel << '\n';
    f << "keyFullscreen=" << keyFullscreen << '\n';
    f << "keyChat=" << keyChat << '\n';
    f << "keyInventory=" << keyInventory << '\n';
    f << "keyThrowBolt=" << keyThrowBolt << '\n';
    f << "keyPlaceSign=" << keyPlaceSign << '\n';
    f << "renderDistance=" << renderDistance << '\n';
    f << "worldSizeIdx=" << worldSizeIdx << '\n';
    f << "invertMouse=" << (invertMouse ? 1 : 0) << '\n';
    f << "soundEnabled=" << (soundEnabled ? 1 : 0) << '\n';
    f << "musicEnabled=" << (musicEnabled ? 1 : 0) << '\n';
    f << "showFps=" << (showFps ? 1 : 0) << '\n';
    f << "viewBobbing=" << (viewBobbing ? 1 : 0) << '\n';
    return true;
}