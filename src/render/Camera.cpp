#include "Camera.hpp"

glm::vec3 Camera::forward() const
{
    float cy = std::cos(glm::radians(yaw));
    float sy = std::sin(glm::radians(yaw));
    float cp = std::cos(glm::radians(pitch));
    float sp = std::sin(glm::radians(pitch));
    return glm::normalize(glm::vec3(sy * cp, sp, -cy * cp));
}

glm::vec3 Camera::right() const
{
    float cy = std::cos(glm::radians(yaw));
    float sy = std::sin(glm::radians(yaw));
    return glm::normalize(glm::vec3(cy, 0.f, sy));
}

glm::mat4 Camera::projection(float aspect)
{
    return glm::perspective(glm::radians(70.f), aspect, 0.05f, 512.f);
}