#ifndef CAVERN_INPUT_HPP
#define CAVERN_INPUT_HPP
#include <GLFW/glfw3.h>

class Input
{
private:
    double lastX = 0.0;
    double lastY = 0.0;
    bool firstMouse = true;
    static Input *inst;

    static void keyCB(GLFWwindow *, int key, int, int action, int);

    static void cursorCB(GLFWwindow *, double x, double y);

public:
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool jump = false;
    bool respawn = false;
    float mouseDX = 0.f;
    float mouseDY = 0.f;

    void init(GLFWwindow *window);

    void beginFrame();
};
#endif//CAVERN_INPUT_HPP