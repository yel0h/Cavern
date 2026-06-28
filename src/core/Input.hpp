#ifndef CAVERN_INPUT_HPP
#define CAVERN_INPUT_HPP
#include <GLFW/glfw3.h>
#include <string>

class Input
{
private:
    GLFWwindow *window = nullptr;
    double lastX = 0.0;
    double lastY = 0.0;
    bool firstMouse = true;
    bool captureIntent = true;
    static Input *inst;
    bool save = false;
    bool spawnMob = false;
    bool slot[8] = {};
    bool toggleFullscreen = false;
    bool cycleFog = false;
    bool newLevel = false;
    bool jumpPressed = false;
    bool pauseToggle = false;
    bool respawn = false;
    bool funcKey[10] = {};
    int scrollDelta = 0;
    bool chatToggle = false;
    bool chatSubmit = false;
    bool chatCancel = false;
    bool hotbarToggle = false;
    bool inventoryToggle = false;
    bool inventoryClick = false;

    static void keyCB(GLFWwindow *, int key, int, int action, int);

    static void charCB(GLFWwindow *, unsigned int codepoint);

    static void cursorCB(GLFWwindow *, double x, double y);

    static void mouseBtnCB(GLFWwindow *, int btn, int action, int);

    static void scrollCB(GLFWwindow *, double dx, double dy);

    static void windowFocusCB(GLFWwindow *, int focused);

public:
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool jump = false;
    float mouseDX = 0.f;
    float mouseDY = 0.f;
    bool mouseCaptured = true;
    bool chatOpen = false;
    std::string chatBuffer;
    bool tabHeld = false;
    bool inventoryOpen = false;
    float mouseX = 0.f;
    float mouseY = 0.f;
    bool invertY = false;
    bool primaryAction = false;
    bool primaryHeld = false;
    bool switchMode = false;
    bool switchHeld = false;

    void init(GLFWwindow *iWindow);

    void beginFrame();

    void setCaptureMode(bool capture);

    bool getPrimaryAction();

    bool getSwitchMode();

    bool getSave();

    bool getSpawnMob();

    bool getSlot(unsigned char index);

    bool getToggleFullscreen();

    bool getCycleFog();

    bool getNewLevel();

    bool getJumpPressed();

    bool getPauseToggle();

    bool getRespawn();

    bool getFuncKey(unsigned char index);

    int getScrollDelta();

    bool getChatToggle();

    bool getChatSubmit();

    bool getChatCancel();

    bool getHotbarToggle();

    bool getInventoryToggle();

    bool getInventoryClick();
};
#endif//CAVERN_INPUT_HPP