#include "Input.hpp"

Input *Input::inst = nullptr;

void Input::init(GLFWwindow *iWindow)
{
    inst = this;
    window = iWindow;
    glfwSetKeyCallback(iWindow, keyCB);
    glfwSetCharCallback(iWindow, charCB);
    glfwSetCursorPosCallback(iWindow, cursorCB);
    glfwSetMouseButtonCallback(iWindow, mouseBtnCB);
    glfwSetScrollCallback(iWindow, scrollCB);
    glfwSetWindowFocusCallback(iWindow, windowFocusCB);
    setCaptureMode(true);
}

void Input::setCaptureMode(bool capture)
{
    captureIntent = capture;
    mouseCaptured = capture;
    if (capture)
    {
        firstMouse = true;
        mouseDX = 0.f;
        mouseDY = 0.f;
    }

    glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (capture && glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

void Input::windowFocusCB(GLFWwindow *window, int focused)
{
    if (!inst)
    {
        return;
    }

    if (!focused)
    {
        inst->mouseCaptured = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else if (inst->captureIntent)
    {
        inst->setCaptureMode(true);
    }
}

void Input::beginFrame()
{
    mouseDX = 0.f;
    mouseDY = 0.f;
}

void Input::keyCB(GLFWwindow *, int key, int, int action, int)
{
    if (!inst)
    {
        return;
    }

    if (inst->captureNextKey)
    {
        if (action == GLFW_PRESS)
        {
            inst->capturedKey = (key == GLFW_KEY_ESCAPE) ? -2 : key;
            inst->captureNextKey = false;
        }

        return;
    }

    if (inst->chatOpen)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            if (key == GLFW_KEY_ENTER)
            {
                inst->chatSubmit = true;
            }

            if (key == GLFW_KEY_ESCAPE)
            {
                inst->chatCancel = true;
            }

            if (key == GLFW_KEY_BACKSPACE && !inst->chatBuffer.empty())
            {
                inst->chatBuffer.pop_back();
            }
        }

        if (key == GLFW_KEY_TAB)
        {
            if (action == GLFW_PRESS)
            {
                inst->tabHeld = true;
            }

            if (action == GLFW_RELEASE)
            {
                inst->tabHeld = false;
            }
        }

        return;
    }

    bool pressed = (action != GLFW_RELEASE);
    bool justPressed  = (action == GLFW_PRESS);
    if (key == inst->bindForward)
    {
        inst->forward = pressed;
    }

    if (key == inst->bindBackward)
    {
        inst->backward = pressed;
    }

    if (key == inst->bindLeft)
    {
        inst->left = pressed;
    }

    if (key == inst->bindRight)
    {
        inst->right = pressed;
    }

    if (key == inst->bindJump)
    {
        inst->jump = pressed;
        if (justPressed)
        {
            inst->jumpPressed = true;
        }
    }

    if (justPressed && key == inst->bindRespawn)
    {
        inst->respawn = true;
    }

    if (justPressed && key == inst->bindSave)
    {
        inst->save = true;
    }

    if (justPressed && key == inst->bindSpawnCrawler)
    {
        inst->spawnMob = true;
    }

    if (justPressed && key == inst->bindCycleFog)
    {
        inst->cycleFog = true;
    }

    if (justPressed && key == inst->bindNewLevel)
    {
        inst->newLevel = true;
    }

    if (justPressed && key == inst->bindFullscreen)
    {
        inst->toggleFullscreen = true;
    }

    if (justPressed && key == inst->bindChat)
    {
        inst->chatToggle = true;
    }

    if (justPressed && key == inst->bindInventory)
    {
        inst->inventoryToggle = true;
    }

    if (justPressed && key == inst->bindThrowBolt)
    {
        inst->throwBolt = true;
    }

    if (justPressed && key == inst->bindPlaceSign)
    {
        inst->placeSign = true;
    }

    switch (key)
    {
        case GLFW_KEY_1:
            if (action == GLFW_PRESS)
            {
                inst->slot[0] = true;
            }

            break;

        case GLFW_KEY_2:
            if (action == GLFW_PRESS)
            {
                inst->slot[1] = true;
            }

            break;

        case GLFW_KEY_3:
            if (action == GLFW_PRESS)
            {
                inst->slot[2] = true;
            }

            break;

        case GLFW_KEY_4:
            if (action == GLFW_PRESS)
            {
                inst->slot[3] = true;
            }

            break;

        case GLFW_KEY_5:
            if (action == GLFW_PRESS)
            {
                inst->slot[4] = true;
            }

            break;

        case GLFW_KEY_6:
            if (action == GLFW_PRESS)
            {
                inst->slot[5] = true;
            }

            break;

        case GLFW_KEY_7:
            if (action == GLFW_PRESS)
            {
                inst->slot[6] = true;
            }

            break;

        case GLFW_KEY_8:
            if (action == GLFW_PRESS)
            {
                inst->slot[7] = true;
            }

            break;

        case GLFW_KEY_TAB:
            if (action == GLFW_PRESS)
            {
                inst->tabHeld = true;
            }

            if (action == GLFW_RELEASE)
            {
                inst->tabHeld = false;
            }

            break;

        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
            {
                inst->pauseToggle = true;
            }

            break;

        case GLFW_KEY_F1:
        case GLFW_KEY_F2:
        case GLFW_KEY_F3:
        case GLFW_KEY_F4:
        case GLFW_KEY_F5:
        case GLFW_KEY_F6:
        case GLFW_KEY_F7:
        case GLFW_KEY_F8:
        case GLFW_KEY_F9:
        case GLFW_KEY_F10:
            if (action == GLFW_PRESS)
            {
                inst->funcKey[key - GLFW_KEY_F1] = true;
            }

            break;

        case GLFW_KEY_H:
            if (action == GLFW_PRESS)
            {
                inst->hotbarToggle = true;
            }

        default:
            break;
    }
}

void Input::charCB(GLFWwindow *, unsigned int codepoint)
{
    if (!inst || !inst->chatOpen)
    {
        return;
    }

    if (inst->chatBuffer.size() < 127 && codepoint < 128)
    {
        inst->chatBuffer += static_cast<char>(codepoint);
    }
}

void Input::cursorCB(GLFWwindow *, double x, double y)
{
    if (!inst)
    {
        return;
    }

    inst->mouseX = (float)x;
    inst->mouseY = (float)y;
    if (inst->firstMouse)
    {
        inst->lastX = x;
        inst->lastY = y;
        inst->firstMouse = false;
        return;
    }

    if (!inst->mouseCaptured)
    {
        inst->lastX = x;
        inst->lastY = y;
        return;
    }

    inst->mouseDX += (float)(x - inst->lastX);
    inst->mouseDY += (float)(y - inst->lastY);
    inst->lastX = x;
    inst->lastY = y;
}

void Input::mouseBtnCB(GLFWwindow *, int btn, int action, int)
{
    if (!inst)
    {
        return;
    }

    if (!inst->mouseCaptured)
    {
        if (inst->inventoryOpen)
        {
            if (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                inst->inventoryClick = true;
            }

            if (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
            {
                inst->primaryHeld = false;
            }

            if (btn == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
            {
                inst->switchHeld = false;
            }

            return;
        }

        if (inst->menuActive)
        {
            if (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                inst->menuClick = true;
            }

            return;
        }

        if (inst->chatOpen && inst->tabHeld)
        {
            if (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                inst->tabListClick = true;
            }

            return;
        }

        return;
    }

    if (btn == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            inst->primaryAction = true;
            inst->primaryHeld = true;
        }

        if (action == GLFW_RELEASE)
        {
            inst->primaryHeld = false;
        }
    }

    if (btn == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            inst->switchMode = true;
            inst->switchHeld = true;
        }

        if (action == GLFW_RELEASE)
        {
            inst->switchHeld = false;
        }
    }
}

void Input::scrollCB(GLFWwindow *, double dx, double dy)
{
    if (!inst)
    {
        return;
    }

    if (!inst->mouseCaptured && !inst->inventoryOpen)
    {
        return;
    }

    inst->scrollDelta += (dy > 0.0) ? 1 : -1;
}

bool Input::getPrimaryAction()
{
    bool temp = primaryAction;
    primaryAction = false;
    return temp;
}

bool Input::getSwitchMode()
{
    bool temp = switchMode;
    switchMode = false;
    return temp;
}

bool Input::getSave()
{
    bool temp = save;
    save = false;
    return temp;
}

bool Input::getSpawnMob()
{
    bool temp = spawnMob;
    spawnMob = false;
    return temp;
}

bool Input::getSlot(unsigned char index)
{
    bool temp = slot[index];
    slot[index] = false;
    return temp;
}

bool Input::getToggleFullscreen()
{
    bool temp = toggleFullscreen;
    toggleFullscreen = false;
    return temp;
}

bool Input::getCycleFog()
{
    bool temp = cycleFog;
    cycleFog = false;
    return temp;
}

bool Input::getNewLevel()
{
    bool temp = newLevel;
    newLevel = false;
    return temp;
}

bool Input::getJumpPressed()
{
    bool temp = jumpPressed;
    jumpPressed = false;
    return temp;
}

bool Input::getPauseToggle()
{
    bool temp = pauseToggle;
    pauseToggle = false;
    return temp;
}

bool Input::getRespawn()
{
    bool temp = respawn;
    respawn = false;
    return temp;
}

bool Input::getFuncKey(unsigned char index)
{
    bool temp = funcKey[index];
    funcKey[index] = false;
    return temp;
}

int Input::getScrollDelta()
{
    int temp = scrollDelta;
    scrollDelta = 0;
    return temp;
}

bool Input::getChatToggle()
{
    bool temp = chatToggle;
    chatToggle = false;
    return temp;
}

bool Input::getChatSubmit()
{
    bool temp = chatSubmit;
    chatSubmit = false;
    return temp;
}

bool Input::getChatCancel()
{
    bool temp = chatCancel;
    chatCancel = false;
    return temp;
}

bool Input::getHotbarToggle()
{
    bool temp = hotbarToggle;
    hotbarToggle = false;
    return temp;
}

bool Input::getInventoryToggle()
{
    bool temp = inventoryToggle;
    inventoryToggle = false;
    return temp;
}

bool Input::getInventoryClick()
{
    bool temp = inventoryClick;
    inventoryClick = false;
    return temp;
}

bool Input::getMenuClick()
{
    bool temp = menuClick;
    menuClick = false;
    return temp;
}

bool Input::getTabListClick()
{
    bool temp = tabListClick;
    tabListClick = false;
    return temp;
}

bool Input::getThrowBolt()
{
    bool temp = throwBolt;
    throwBolt = false;
    return temp;
}

bool Input::getPlaceSign()
{
    bool temp = placeSign;
    placeSign = false;
    return temp;
}