#ifndef CAVERN_RENDERER_HPP
#define CAVERN_RENDERER_HPP
#include "Camera.hpp"
#include "ChunkMesh.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include <array>

class World;

class Renderer
{
private:
    Shader shader;
    TextureAtlas atlas;
    std::array<ChunkMesh, 256> meshes;

    static constexpr const char *vertSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in float aLight;

uniform mat4 uMVP;
uniform float uFogNear;
uniform float uFogFar;

out vec2 vUV;
out float vLight;
out float vFogFactor;
out float vEyeDist;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
    vUV = aUV;
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
in vec2 vUV;
in float vLight;
in float vFogFactor;
in float vEyeDist;

uniform sampler2D uAtlas;
uniform vec3 uFogColor;
uniform float uFogNear;
uniform float uFogFar;

out vec4 fragColor;

void main()
{
    vec4 tex = texture(uAtlas, vUV);
    if (tex.a < 0.1)
    {
        discard;
    }

    vec3 lit = tex.rgb * mix(0.25, 1.0, vLight);
    vec3 first = mix(mix(vec3(0.1), uFogColor, vLight), lit, vFogFactor);
    float secondFactor = clamp((uFogFar - vEyeDist) / (uFogFar - uFogNear), 0.0, 1.0);
    vec3 final = mix(uFogColor, first, mix(secondFactor, 1.0, vLight));
    fragColor = vec4(final, 1.0);
}
)";

    void rebuildDirty(const World &world);

public:
    void init();

    void shutdown();

    void renderFrame(const World &world, const Camera &cam, int winW, int winH);
};
#endif//CAVERN_RENDERER_HPP