#ifndef CAVERN_SETTINGS_HPP
#define CAVERN_SETTINGS_HPP
#include "GLFW/glfw3.h"

struct Settings
{
    int keyForward = GLFW_KEY_W;
    int keyBackward = GLFW_KEY_S;
    int keyLeft = GLFW_KEY_A;
    int keyRight = GLFW_KEY_D;
    int keyJump = GLFW_KEY_SPACE;
    int keySave = GLFW_KEY_ENTER;
    int keyCycleFog = GLFW_KEY_F;
    int keyNewLevel = GLFW_KEY_N;
    int keyFullscreen = GLFW_KEY_F11;
    int keyChat = GLFW_KEY_T;
    int keyInventory = GLFW_KEY_B;
    int keyThrowBolt = GLFW_KEY_V;
    int keyPlaceSign = GLFW_KEY_H;
    int renderDistance = 2;
    int worldSizeIdx = 2;
    bool invertMouse = false;
    bool soundEnabled = true;
    bool musicEnabled = true;
    bool showFps = true;
    bool viewBobbing = false;

    bool load(const char *path = "settings.cfg");

    bool save(const char *path = "settings.cfg") const;
};
#endif//CAVERN_SETTINGS_HPP