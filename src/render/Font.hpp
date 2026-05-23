#ifndef CAVERN_FONT_HPP
#define CAVERN_FONT_HPP
class Font
{
private:
    static const unsigned char glyphs[128][8];

public:
    static constexpr int CHAR_W = 8;
    static constexpr int CHAR_H = 8;
    static constexpr int ATLAS_COLS = 16;
    static constexpr int ATLAS_ROWS = 8;
    unsigned int texId = 0;

    void init();

    void shutdown();

    void uvForChar(char c, float &u0, float &v0, float &u1, float &v1) const;
};
#endif//CAVERN_FONT_HPP