#include "Input.hpp"

Input *Input::inst = nullptr;

void Input::init(GLFWwindow *iWindow)
{
    inst = this;
    window = iWindow;
    glfwSetKeyCallback(iWindow, keyCB);
    glfwSetCursorPosCallback(iWindow, cursorCB);
    glfwSetMouseButtonCallback(iWindow, mouseBtnCB);
    glfwSetInputMode(iWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(iWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
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

    bool pressed = (action != GLFW_RELEASE);
    switch (key)
    {
        case GLFW_KEY_W:
            inst->forward = pressed;
            break;

        case GLFW_KEY_S:
            inst->backward = pressed;
            break;

        case GLFW_KEY_A:
            inst->left = pressed;
            break;

        case GLFW_KEY_D:
            inst->right = pressed;
            break;

        case GLFW_KEY_SPACE:
            inst->jump = pressed;
            if (action == GLFW_PRESS)
            {
                inst->jumpPressed = true;
            }

            break;

        case GLFW_KEY_R:
            if (action == GLFW_PRESS)
            {
                inst->respawn = true;
            }

            break;

        case GLFW_KEY_ENTER:
            if (action == GLFW_PRESS)
            {
                inst->save = true;
            }

            break;

        case GLFW_KEY_G:
            if (action == GLFW_PRESS)
            {
                inst->spawnMob = true;
            }

            break;

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

        case GLFW_KEY_F:
            if (action == GLFW_PRESS)
            {
                inst->cycleFog = true;
            }

            break;

        case GLFW_KEY_N:
            if (action == GLFW_PRESS)
            {
                inst->newLevel = true;
            }

            break;

        case GLFW_KEY_F11:
            if (action == GLFW_PRESS)
            {
                inst->toggleFullscreen = true;
            }

            break;

        case GLFW_KEY_Y:
            if (action == GLFW_PRESS)
            {
                inst->invertY = !inst->invertY;
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

        default:
            break;
    }
}

void Input::cursorCB(GLFWwindow *, double x, double y)
{
    if (!inst)
    {
        return;
    }

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
    if (!inst || action != GLFW_PRESS)
    {
        return;
    }

    if (!inst->mouseCaptured)
    {
        inst->mouseCaptured = true;
        glfwSetInputMode(inst->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        inst->firstMouse = true;
        return;
    }

    if (btn == GLFW_MOUSE_BUTTON_LEFT)
    {
        inst->primaryAction = true;
    }

    if (btn == GLFW_MOUSE_BUTTON_RIGHT)
    {
        inst->switchMode = true;
    }
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