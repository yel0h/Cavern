#include "Input.hpp"

Input *Input::inst = nullptr;

void Input::init(GLFWwindow *window)
{
    inst = this;
    glfwSetKeyCallback(window, keyCB);
    glfwSetCursorPosCallback(window, cursorCB);
    glfwSetMouseButtonCallback(window, mouseBtnCB);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
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
            break;

        case GLFW_KEY_R:
            inst->respawn = pressed;
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

        case GLFW_KEY_F11:
            if (action == GLFW_PRESS)
            {
                inst->toggleFullscreen = true;
            }

            break;

        case GLFW_KEY_ESCAPE:
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

    if (btn == GLFW_MOUSE_BUTTON_RIGHT)
    {
        inst->placeBlock = true;
    }

    if (btn == GLFW_MOUSE_BUTTON_LEFT)
    {
        inst->destroyBlock = true;
    }
}

bool Input::getPlaceBlock()
{
    bool temp = placeBlock;
    placeBlock = false;
    return temp;
}

bool Input::getDestroyBlock()
{
    bool temp = destroyBlock;
    destroyBlock = false;
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