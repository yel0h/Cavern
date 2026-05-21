#include "Input.hpp"

Input *Input::inst = nullptr;

void Input::init(GLFWwindow *window)
{
    inst = this;
    glfwSetKeyCallback(window, keyCB);
    glfwSetCursorPosCallback(window, cursorCB);
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