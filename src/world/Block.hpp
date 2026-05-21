#ifndef CAVERN_GAME_BLOCK_HPP
#define CAVERN_GAME_BLOCK_HPP
enum class BlockType : unsigned char
{
    Air = 0,
    Turf = 1,
    Stone = 2,
};

struct BlockDef
{
    bool opaque;
    bool transparent;
    unsigned char texTop;
    unsigned char texSide;
    unsigned char texBottom;
};

inline constexpr BlockDef blockDefs[3] = {
        {false, true,  0, 0, 0},
        {true, false, 1, 2, 3},
        {true, false, 4, 4, 4},
};

inline constexpr const BlockDef &blockDef(BlockType t)
{
    return blockDefs[static_cast<unsigned char>(t)];
}
#endif//CAVERN_GAME_BLOCK_HPP