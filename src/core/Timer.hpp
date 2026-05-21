#ifndef CAVERN_TIMER_HPP
#define CAVERN_TIMER_HPP
#include <GLFW/glfw3.h>
#include <algorithm>

struct Timer
{
    static constexpr double dt = 1.0 / 60.0;
    double last  = 0.0;
    double accum = 0.0;

    void start() { last = glfwGetTime(); }

    int advance()
    {
        double now = glfwGetTime();
        double elapsed = std::min(now - last, 0.25);
        last = now;
        accum += elapsed;
        int ticks = 0;
        while (accum >= dt)
        {
            accum -= dt;
            ticks++;
        }

        return ticks;
    }
};
#endif//CAVERN_TIMER_HPP