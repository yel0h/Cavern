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
    Bedrock = 8,
    Lava = 9,
    Water = 10,
    Silt = 11,
    Grit = 12,
    CharVein = 13,
    IronVein = 14,
    GoldVein = 15,
    Glaze = 16,
    Pith = 17,
};

struct BlockDef
{
    bool opaque;
    bool transparent;
    bool liquid;
    unsigned char texTop;
    unsigned char texSide;
    unsigned char texBottom;
};

inline constexpr BlockDef blockDefs[18] = {
        {false, true, false, 0, 0, 0},
        {true, false, false, 1, 2, 3},
        {true, false, false, 4, 4, 4},
        {true, false, false, 5, 5, 5},
        {true, false, false, 3, 3, 3},
        {true, false, false, 6, 6, 6},
        {false, true, false, 7, 7, 7},
        {true, false, false, 8, 8, 8},
        {true, false, false, 9, 9, 9},
        {false, false, true, 10, 10, 10},
        {false, true, true, 11, 11, 11},
        {true, false, false, 12, 12, 12},
        {true, false, false, 13, 13, 13},
        {true, false, false, 14, 14, 14},
        {true, false, false, 15, 15, 15},
        {true, false, false, 16, 16, 16},
        {false, true, false, 17, 17, 17},
        {true, false, false, 18, 18, 18},
};

inline constexpr const BlockDef &blockDef(BlockType t)
{
    return blockDefs[static_cast<unsigned char>(t)];
}
#endif//CAVERN_GAME_BLOCK_HPP