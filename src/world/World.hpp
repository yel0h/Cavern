#ifndef CAVERN_WORLD_HPP
#define CAVERN_WORLD_HPP
#include "Chunk.hpp"
#include <memory>
#include <vector>

class World
{
private:
    unsigned int tick = 0;
    std::vector<std::tuple<int, int, int, unsigned int>> wickTimers;

    static int chunkIdx(int cx, int cz) { return (cz * CHUNKS_X) + cx; }

    void absorbLiquids(int wx, int wy, int wz);

public:
    static constexpr int CHUNKS_X = 16;
    static constexpr int CHUNKS_Z = 16;
    static constexpr int BLOCK_W = CHUNKS_X * Chunk::WIDTH;
    static constexpr int BLOCK_H = Chunk::HEIGHT;
    static constexpr int BLOCK_D = CHUNKS_Z * Chunk::DEPTH;
    std::array<std::unique_ptr<Chunk>, CHUNKS_X * CHUNKS_Z> chunks;

    World();

    Chunk *getChunk(int cx, int cz);

    [[nodiscard]] const Chunk *getChunk(int cx, int cz) const;

    [[nodiscard]] static bool inBounds(int wx, int wy, int wz);

    [[nodiscard]] BlockType getBlock(int wx, int wy, int wz) const;

    void setBlock(int wx, int wy, int wz, BlockType t);

    [[nodiscard]] unsigned char getLight(int wx, int wy, int wz) const;

    void setLight(int wx, int wy, int wz, unsigned char v);

    void generate(unsigned int seed);

    void tickDynamic();

    bool save(const char *path) const;

    bool load(const char *path);
};
#endif//CAVERN_WORLD_HPP