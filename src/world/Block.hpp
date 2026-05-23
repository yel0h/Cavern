#ifndef CAVERN_GAME_BLOCK_HPP
#define CAVERN_GAME_BLOCK_HPP
enum class BlockType : unsigned char
{
    Air = 0,
    Turf = 1,
    Stone = 2,
    Rubble = 3,
    Soil = 4,
    Timber = 5,
    Sapling = 6,
    Boards = 7,
};

struct BlockDef
{
    bool opaque;
    bool transparent;
    unsigned char texTop;
    unsigned char texSide;
    unsigned char texBottom;
};

inline constexpr BlockDef blockDefs[8] = {
        {false, true,  0, 0, 0},
        {true, false, 1, 2, 3},
        {true, false, 4, 4, 4},
        {true, false, 5, 5, 5},
        {true, false, 3, 3, 3},
        {true, false, 6, 6, 6},
        {false, true,  7, 7, 7},
        {true, false, 8, 8, 8},
};

inline constexpr const BlockDef &blockDef(BlockType t)
{
    return blockDefs[static_cast<unsigned char>(t)];
}
#endif//CAVERN_GAME_BLOCK_HPP