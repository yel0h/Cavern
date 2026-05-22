#ifndef CAVERN_RENDERER_HPP
#define CAVERN_RENDERER_HPP
#include "Camera.hpp"
#include "ChunkMesh.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include "src/world/Block.hpp"
#include <array>

class World;

class Renderer
{
public:
    struct HighlightFace
    {
        bool valid = false;
        int bx = 0;
        int by = 0;
        int bz = 0;
        int face = 0;
    };

private:
    Shader shader;
    Shader hlShader;
    Shader _2dShader;
    Shader hudShader;
    TextureAtlas atlas;
    std::array<ChunkMesh, 256> meshes;
    unsigned int hlVao = 0;
    unsigned int hlVbo = 0;
    unsigned int xhVao = 0;
    unsigned int xhVbo = 0;
    unsigned int hudVao = 0;
    unsigned int hudVbo = 0;
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

    vec3 lit = tex.rgb * mix(0.45, 1.0, vLight);
    vec3 first = mix(mix(vec3(0.1), uFogColor, vLight), lit, vFogFactor);
    float secondFactor = clamp((uFogFar - vEyeDist) / (uFogFar - uFogNear), 0.0, 1.0);
    vec3 final = mix(uFogColor, first, mix(secondFactor, 1.0, vLight));
    fragColor = vec4(final, 1.0);
}
)";
    static constexpr const char *hlVertSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;

uniform mat4 uMVP;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";
    static constexpr const char *hlFragSrc = R"(
#version 330 core
uniform float uTime;

out vec4 fragColor;

void main()
{
    float a = 0.25 + (0.25 * abs(sin(uTime * 5.0)));
    fragColor = vec4(1.0, 1.0, 1.0, a);
}
)";
    static constexpr const char *crosshairVertSrc = R"(
#version 330 core
layout(location=0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";
    static constexpr const char *crosshairFragSrc = R"(
#version 330 core
out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0, 1.0, 1.0, 0.8);
}
)";
    static constexpr const char *hudVertSrc = R"(
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;

out vec2 vUV;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
)";
    static constexpr const char *hudFragSrc = R"(
#version 330 core
in vec2 vUV;

uniform sampler2D uAtlas;

out vec4 fragColor;

void main()
{
    fragColor = texture(uAtlas, vUV);
}
)";

    void rebuildDirty(const World &world);

    void initHighlight();

    void renderHighlight(const HighlightFace& hl, const float* vp, float time);

    void initCrosshair();

    void renderCrosshair(int winW, int winH);

    void initHUD();

    void renderHUD(int winW, int winH, BlockType selectedBlock);

public:
    void init();

    void shutdown();

    void renderFrame(const World &world, const Camera &cam, int winW, int winH,
                     const HighlightFace &hl, float time,
                     BlockType selectedBlock);
};
#endif//CAVERN_RENDERER_HPP