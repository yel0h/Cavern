#ifndef CAVERN_MOBRENDERER_HPP
#define CAVERN_MOBRENDERER_HPP
#include "Shader.hpp"
#include <vector>
class MobManager;
class Camera;

class MobRenderer
{
private:
    Shader shader;
    unsigned int vao = 0;
    unsigned int vbo = 0;

    struct MobVertex
    {
        float x;
        float y;
        float z;
        float r;
        float g;
        float b;
        float light;
    };

    std::vector<MobVertex> verts;

    void addBox(float x0, float y0, float z0,
                float x1, float y1, float z1,
                float br, float bg, float bb,
                float light,
                float yawDeg, float wx, float wy, float wz,
                float pvtX, float pvtY, float pvtZ, float rotAngle);

    void buildSnout(float wx, float wy, float wz, float yawDeg, float legA, float legB, float light);

    void buildBoneshade(float wx, float wy, float wz, float yawDeg, float armA, float legA, float light);

    void buildGrubbin(float wx, float wy, float wz, float yawDeg, float armA, float legA, float light);

    void buildFumewretch(float wx, float wy, float wz, float yawDeg, float bob, float flashAmt, float light);

    static constexpr const char *vertSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
layout(location=2) in float aLight;

uniform mat4 uMVP;
uniform float uFogNear;
uniform float uFogFar;

out vec3 vColor;
out float vLight;
out float vFogFactor;
out float vEyeDist;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
    vColor = aColor;
    vLight = aLight;
    float eyeDist = length((uMVP * vec4(aPos, 1.0)).xyz);
    vEyeDist = eyeDist;
    float fogNear = mix(4.0, uFogNear, aLight);
    float fogFar = mix(12.0, uFogFar, aLight);
    vFogFactor = clamp((fogFar - eyeDist) / (fogFar - fogNear), 0.0, 1.0);
}
)";

    static constexpr const char *fragSrc = R"(
#version 330 core
in vec3 vColor;
in float vLight;
in float vFogFactor;
in float vEyeDist;

uniform vec3 uFogColor;
uniform float uFogNear;
uniform float uFogFar;

out vec4 fragColor;

void main()
{
    vec3 lit = vColor * mix(0.45, 1.0, vLight);
    vec3 first = mix(mix(vec3(0.1), uFogColor, vLight), lit, vFogFactor);
    float secondFactor = clamp((uFogFar - vEyeDist) / (uFogFar - uFogNear), 0.0, 1.0);
    vec3 final = mix(uFogColor, first, mix(secondFactor, 1.0, vLight));
    fragColor  = vec4(final, 1.0);
}
)";

public:
    void init();

    void shutdown();

    void render(const MobManager &mgr, const Camera &cam, int winW, int winH, float time);
};
#endif//CAVERN_MOBRENDERER_HPP