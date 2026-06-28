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
    GoldBlock = 18,
    WeavePale = 19,
    WeaveAsh = 20,
    WeaveSlate = 21,
    WeaveRust = 22,
    WeaveBurn = 23,
    WeaveGlow = 24,
    WeaveBlight = 25,
    WeaveMold = 26,
    WeaveFern = 27,
    WeaveFrost = 28,
    WeaveAzure = 29,
    WeaveDeep = 30,
    WeaveDusk = 31,
    WeaveMurk = 32,
    WeaveBloom = 33,
    WeaveBlush = 34,
    Goldenbloom = 35,
    Thornbloom = 36,
    Dustshroom = 37,
    Emberscap = 38,
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

inline constexpr BlockDef blockDefs[39] = {
        {false, true, false, 0, 0, 0},
        {true, false, false, 1, 2, 3},
        {true, false, false, 4, 4, 4},
        {true, false, false, 5, 5, 5},
        {true, false, false, 3, 3, 3},
        {true, false, false, 6, 6, 6},
        {false, false, false, 7, 7, 7},
        {true, false, false, 8, 8, 8},
        {true, false, false, 9, 9, 9},
        {false, false, true, 10, 10, 10},
        {false, true, true, 11, 11, 11},
        {true, false, false, 12, 12, 12},
        {true, false, false, 13, 13, 13},
        {true, false, false, 14, 14, 14},
        {true, false, false, 15, 15, 15},
        {true, false, false, 16, 16, 16},
        {false, false, false, 17, 17, 17},
        {true, false, false, 18, 18, 18},
        {true, false, false, 19, 19, 19},
        {true, false, false, 20, 20, 20},
        {true, false, false, 21, 21, 21},
        {true, false, false, 22, 22, 22},
        {true, false, false, 23, 23, 23},
        {true, false, false, 24, 24, 24},
        {true, false, false, 25, 25, 25},
        {true, false, false, 26, 26, 26},
        {true, false, false, 27, 27, 27},
        {true, false, false, 28, 28, 28},
        {true, false, false, 29, 29, 29},
        {true, false, false, 30, 30, 30},
        {true, false, false, 31, 31, 31},
        {true, false, false, 32, 32, 32},
        {true, false, false, 33, 33, 33},
        {true, false, false, 34, 34, 34},
        {true, false, false, 35, 35, 35},
        {false, false, false, 36, 36, 36},
        {false, false, false, 37, 37, 37},
        {false, false, false, 38, 38, 38},
        {false, false, false, 39, 39, 39},
};

inline constexpr const BlockDef &blockDef(BlockType t)
{
    return blockDefs[static_cast<unsigned char>(t)];
}
#endif//CAVERN_GAME_BLOCK_HPP