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
    bool throwBolt = false;
    bool placeSign = false;
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
    bool menuClick = false;
    bool tabListClick = false;

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
    int bindForward = GLFW_KEY_W;
    int bindBackward = GLFW_KEY_S;
    int bindLeft = GLFW_KEY_A;
    int bindRight = GLFW_KEY_D;
    int bindJump = GLFW_KEY_SPACE;
    int bindRespawn = GLFW_KEY_R;
    int bindSave = GLFW_KEY_ENTER;
    int bindSpawnCrawler = GLFW_KEY_G;
    int bindCycleFog = GLFW_KEY_F;
    int bindNewLevel = GLFW_KEY_N;
    int bindFullscreen = GLFW_KEY_F11;
    int bindChat = GLFW_KEY_T;
    int bindInventory = GLFW_KEY_B;
    int bindThrowBolt = GLFW_KEY_V;
    int bindPlaceSign = GLFW_KEY_H;
    bool captureNextKey = false;
    int capturedKey = -1;
    bool menuActive = false;
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

    bool getMenuClick();

    bool getTabListClick();

    bool getThrowBolt();

    bool getPlaceSign();
};
#endif//CAVERN_INPUT_HPP