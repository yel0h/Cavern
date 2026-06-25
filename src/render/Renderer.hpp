#ifndef CAVERN_RENDERER_HPP
#define CAVERN_RENDERER_HPP
#include "Camera.hpp"
#include "ChunkMesh.hpp"
#include "Font.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include "src/net/NetTypes.hpp"
#include "src/world/Block.hpp"
#include <array>

class World;

class Renderer
{
public:
    struct HighlightBlock
    {
        bool valid = false;
        int bx = 0;
        int by = 0;
        int bz = 0;
    };

    static constexpr const char *version = "0.4.3";
    int lastChunkUpdates = 0;

    void init();

    void shutdown();

    void renderFrame(const World &world, const Camera &cam, int winW, int winH,
                     const HighlightBlock &hl, float time,
                     BlockType selectedBlock, int fps, int chunkUpdates, bool placeMode,
                     bool underLava, bool underWater);

    void renderGenerating(int winW, int winH);

    void renderPauseMenu(int winW, int winH);

    void renderPlayerNames(const std::vector<RemotePlayer> &players, const Camera &cam, int winW, int winH);

    void renderChat(const std::vector<std::string> &msgs, bool chatOpen, const std::string &buffer, int winW, int winH);

    void renderPlayerList(const std::vector<std::string> &names, int winW, int winH);

    void markAllDirty() { for (auto &m : meshes) m.free(); }

    void cycleFog() { fogLevel = (fogLevel + 1) % 4; }

private:
    Shader shader;
    Shader hlShader;
    Shader olShader;
    Shader _2dShader;
    Shader hudShader;
    Shader txtShader;
    TextureAtlas atlas;
    Font font;
    std::array<ChunkMesh, 256> meshes;
    unsigned int hlVao = 0;
    unsigned int hlVbo = 0;
    unsigned int olVao = 0;
    unsigned int olVbo = 0;
    unsigned int xhVao = 0;
    unsigned int xhVbo = 0;
    unsigned int hudVao = 0;
    unsigned int hudVbo = 0;
    unsigned int txtVao = 0;
    unsigned int txtVbo = 0;
    unsigned int cloudVao = 0;
    unsigned int cloudVbo = 0;
    Shader cloudShader;
    int cloudVertCount = 0;
    int fogLevel = 2;
    static constexpr int maxRebuildsPerFrame = 4;
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
    vec4 clipPos = uMVP * vec4(aPos, 1.0);
    gl_Position = clipPos;
    vUV = aUV;
    vLight = aLight;
    float eyeDist = clipPos.w;
    vEyeDist = eyeDist;
    vFogFactor = clamp((uFogFar - eyeDist) / (uFogFar - uFogNear), 0.0, 1.0);
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

    float distDarken = mix(0.88, 1.0, vFogFactor);
    vec3 lit = tex.rgb * mix(0.45, 1.0, vLight) * distDarken;
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
    static constexpr const char *olFragSrc = R"(
#version 330 core
out vec4 fragColor;

void main()
{
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
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
    static constexpr const char *txtVertSrc = R"(
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
    static constexpr const char *txtFragSrc = R"(
#version 330 core
in vec2 vUV;

uniform sampler2D uFont;
uniform vec3 uColor;

out vec4 fragColor;

void main()
{
    float a = texture(uFont, vUV).a;
    fragColor = vec4(uColor, a);
}
)";
    static constexpr const char *cloudVertSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;

uniform mat4 uVP;
uniform float uDrift;

void main()
{
    gl_Position = uVP * vec4(aPos.x + uDrift, aPos.y, aPos.z, 1.0);
}
)";
    static constexpr const char *cloudFragSrc = R"(
#version 330 core
out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0, 1.0, 1.0, 0.72);
}
)";

    void rebuildDirty(const World &world, const glm::vec3 &playerPos);

    void initClouds();

    void renderClouds(const float *vp, float time);

    void initHighlight();

    void renderHighlight(const HighlightBlock &hl, const float *vp, float time, const World &world);

    void initOutline();

    void renderOutline(const HighlightBlock &hl, const float *vp);

    void initCrosshair();

    void renderCrosshair(int winW, int winH);

    void initHUD();

    void renderHUD(int winW, int winH, BlockType selectedBlock);

    void initText();

    void drawText(const char *text, float px, float py, int scale, int winW, int winH) const;

    void renderDebug(int winW, int winH, int fps, int chunkUpdates, bool placeMode);
};
#endif//CAVERN_RENDERER_HPP