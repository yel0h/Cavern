#ifndef CAVERN_CAMERA_HPP
#define CAVERN_CAMERA_HPP
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    glm::vec3 position{128.f, 72.f, 128.f};
    float yaw = 0.f;
    float pitch = 0.f;

    [[nodiscard]] glm::mat4 view() const { return glm::lookAt(position, position + forward(), glm::vec3(0.f, 1.f, 0.f)); }

    [[nodiscard]] static glm::mat4 projection(float aspect);

    [[nodiscard]] glm::mat4 viewProjection(float aspect) const { return projection(aspect) * view(); }

    [[nodiscard]] glm::vec3 forward() const;

    [[nodiscard]] glm::vec3 right() const;
};
#endif//CAVERN_CAMERA_HPP