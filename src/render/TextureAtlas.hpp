#ifndef CAVERN_TEXTUREATLAS_HPP
#define CAVERN_TEXTUREATLAS_HPP
#include <algorithm>
#include <glad/glad.h>

class TextureAtlas
{
private:
    static void paintTile(unsigned int *pixels, int tile);

    static unsigned int ph(int x, int y, int tile);

    static unsigned int clampByte(int v) { return (unsigned int)std::clamp(v, 0, 255); }

    static unsigned int makePixel(int r, int g, int b);

public:
    static constexpr int TILE_SIZE = 16;
    static constexpr int TILE_COUNT = 5;
    static constexpr int WIDTH = TILE_SIZE * TILE_COUNT;
    static constexpr int HEIGHT = TILE_SIZE;
    unsigned int texId = 0;

    void build();

    void bind(int unit = 0) const;

    static void uvRect(int tile, float &u0, float &v0, float &u1, float &v1);
};
#endif//CAVERN_TEXTUREATLAS_HPP