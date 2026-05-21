#ifndef CAVERN_GAME_CHUNK_HPP
#define CAVERN_GAME_CHUNK_HPP
#include "Block.hpp"
#include <array>

class Chunk
{
public:
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 64;
    static constexpr int DEPTH = 16;
    int x = 0;
    int z = 0;
    bool dirty = true;
    bool generated = false;
    std::array<BlockType, WIDTH * HEIGHT * DEPTH> blocks{};
    std::array<unsigned char, WIDTH * HEIGHT * DEPTH> light{};

    static int idx(int x, int y, int z) { return (y * WIDTH * DEPTH) + (z * WIDTH) + x; }

    [[nodiscard]] BlockType get(int bx, int by, int bz) const { return blocks[idx(bx, by, bz)]; }

    void set(int bx, int by, int bz, BlockType t) { blocks[idx(bx, by, bz)] = t; }

    [[nodiscard]] unsigned char getLight(int bx, int by, int bz) const { return light[idx(bx, by, bz)]; }

    void setLight(int bx, int by, int bz, unsigned char v) { light[idx(bx, by, bz)] = v; }
};
#endif//CAVERN_GAME_CHUNK_HPP