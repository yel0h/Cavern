#ifndef CAVERN_INPUT_HPP
#define CAVERN_INPUT_HPP
#include <GLFW/glfw3.h>

class Input
{
private:
    GLFWwindow *window = nullptr;
    double lastX = 0.0;
    double lastY = 0.0;
    bool firstMouse = true;
    static Input *inst;
    bool primaryAction = false;
    bool switchMode = false;
    bool save = false;
    bool spawnMob = false;
    bool slot[6] = {};
    bool toggleFullscreen = false;

    static void keyCB(GLFWwindow *, int key, int, int action, int);

    static void cursorCB(GLFWwindow *, double x, double y);

    static void mouseBtnCB(GLFWwindow *, int btn, int action, int);

public:
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool jump = false;
    bool respawn = false;
    float mouseDX = 0.f;
    float mouseDY = 0.f;
    bool mouseCaptured = true;
    bool invertY = false;

    void init(GLFWwindow *iWindow);

    void beginFrame();

    bool getPrimaryAction();

    bool getSwitchMode();

    bool getSave();

    bool getSpawnMob();

    bool getSlot(unsigned char index);

    bool getToggleFullscreen();
};
#endif//CAVERN_INPUT_HPP