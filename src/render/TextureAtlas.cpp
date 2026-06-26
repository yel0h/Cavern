#include "TextureAtlas.hpp"
#include <cmath>
#include <vector>

unsigned int TextureAtlas::ph(int x, int y, int tile)
{
    return (unsigned int)(x * 2654435761u) ^ (unsigned int)(y * 805459861u) ^ (unsigned int)(tile * 1234567u);
}

unsigned int TextureAtlas::makePixel(int r, int g, int b)
{
    return 0xFF000000u | clampByte(b) << 16 | clampByte(g) << 8 | clampByte(r);
}

void TextureAtlas::paintTile(unsigned int *pixels, int tile)
{
    for (int y = 0; y < TILE_SIZE; y++)
    {
        for (int x = 0; x < TILE_SIZE; x++)
        {
            unsigned int h = ph(x, y, tile);
            unsigned int &px = pixels[(y * WIDTH) + (tile * TILE_SIZE) + x];
            switch (tile)
            {
                case 0:
                    px = 0x00000000u;
                    break;

                case 1:
                {
                    int r = 60 + (int)((ph(x, y, 1) & 0x1F) - 15);
                    int g = 145 + (int)((ph(x, y, 11) & 0x1F) - 15);
                    int b = 40 + (int)((ph(x, y, 111) & 0x1F) - 15);
                    if (ph(x, y, 2) % 20 == 0)
                    {
                        r = 100;
                        g = 200;
                        b = 70;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 2:
                {
                    if (y < 3)
                    {
                        int r = 60 + (int)((ph(x, y, 1) & 0x1F) - 15);
                        int g = 145 + (int)((ph(x, y, 11) & 0x1F) - 15);
                        int b = 40 + (int)((ph(x, y, 111) & 0x1F) - 15);
                        if (ph(x, y, 2) % 20 == 0)
                        {
                            r = 100;
                            g = 200;
                            b = 70;
                        }

                        px = makePixel(r, g, b);
                    }
                    else
                    {
                        int r = 124 + (int)((ph(x, y, 22) & 0x17) - 12);
                        int g = 68 + (int)((ph(x, y, 222) & 0x17) - 12);
                        int b = 20 + (int)((ph(x, y, 2222) & 0x17) - 12);
                        px = makePixel(r, g, b);
                    }

                    break;
                }

                case 3:
                {
                    int r = 124 + (int)((ph(x, y, 33) & 0x17) - 12);
                    int g = 68 + (int)((ph(x, y, 333) & 0x17) - 12);
                    int b = 20 + (int)((ph(x, y, 3333) & 0x17) - 12);
                    px = makePixel(r, g, b);
                    break;
                }

                case 4:
                {
                    int base = 120 + (int)((h & 0x27) - 20);
                    int r = base - 8;
                    int g = base - 4;
                    int b = base + 14;
                    if (ph(x, y, 5) % 18 == 0)
                    {
                        r += 20;
                        g += 20;
                        b += 20;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 5:
                {
                    int base = 100 + (int)((h & 0x27) - 20);
                    int r = base;
                    int g = base;
                    int b = base;
                    if (ph(x, y, 777) % 9 == 0 || ph(x ^ y, x + y, 5) % 11 == 0)
                    {
                        r = 55; g = 55; b = 55;
                    }
                    else if (ph(x, y, 55) % 12 == 0)
                    {
                        unsigned int fleck = ph(x, y, 555);
                        int ch = (int)(fleck % 3);
                        int delta = (int)((fleck >> 2) & 0x3F) - 30;
                        if (ch == 0)
                        {
                            r += delta;
                        }
                        else if (ch == 1)
                        {
                            g += delta;
                        }
                        else
                        {
                            b += delta;
                        }
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 6:
                {
                    float fdx = (float)x - 7.5f;
                    float fdy = (float)y - 7.5f;
                    int ringIdx = (int)(std::sqrt((fdx * fdx) + (fdy * fdy)) * 1.5f);
                    int rm = ringIdx % 3;
                    int r = (rm == 0) ? 155 : (rm == 1) ? 172 : 185;
                    int g = (rm == 0) ? 100 : (rm == 1) ? 115 : 128;
                    int b = (rm == 0) ? 42 : (rm == 1) ? 52 : 60;
                    int noise = (int)((ph(x, y, 61) & 0x0F) - 8);
                    r = std::clamp(r + noise, 0, 255);
                    g = std::clamp(g + noise, 0, 255);
                    b = std::clamp(b + (noise / 2), 0, 255);
                    px = makePixel(r, g, b);
                    break;
                }

                case 7:
                {
                    if (ph(x, y, 70) % 4 == 0)
                    {
                        px = 0x00000000u;
                    }
                    else
                    {
                        int shade = (int)(ph(x, y, 71) % 3);
                        int r = (shade == 0) ? 35 : (shade == 1) ? 55 : 75;
                        int g = (shade == 0) ? 110 : (shade == 1) ? 145 : 170;
                        int b = (shade == 0) ? 20 : (shade == 1) ? 30 : 45;
                        int noise = (int)((ph(x, y, 72) & 0x0F) - 8);
                        r = std::clamp(r + noise, 0, 255);
                        g = std::clamp(g + (noise * 2), 0, 255);
                        b = std::clamp(b + noise, 0, 255);
                        px = makePixel(r, g, b) | 0xFF000000u;
                    }

                    break;
                }

                case 8:
                {
                    int r = 200 + (int)((ph(x, y, 81) & 0x0F) - 8);
                    int g = 165 + (int)((ph(x, y, 82) & 0x0F) - 8);
                    int b = 95 + (int)((ph(x, y, 83) & 0x0F) - 8);
                    int row = y / 4;
                    int offset = (row % 2) * 8;
                    int seam = (x + offset) % 8;
                    if (y % 4 == 0)
                    {
                        r = r * 55 / 100;
                        g = g * 55 / 100;
                        b = b * 55 / 100;
                    }
                    else if (seam == 0)
                    {
                        r = r * 60 / 100;
                        g = g * 60 / 100;
                        b = b * 60 / 100;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 9:
                {
                    int base = 38 + (int)((h & 0x1F) - 16);
                    int r = base;
                    int g = base;
                    int b = base;
                    if (ph(x, y, 91) % 7 == 0)
                    {
                        r += 22;
                        g += 22;
                        b += 22;
                    }

                    if (ph(x, y, 92) % 13 == 0)
                    {
                        r -= 10;
                        g -= 10;
                        b -= 10;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 10:
                {
                    int r = 235 + (int)((ph(x, y, 101) & 0x1F) - 20);
                    int g = 70 + (int)((ph(x, y, 102) & 0x1F) - 20);
                    int b = 10 + (int)((ph(x, y, 103) & 0x0F) - 8);
                    if (ph(x, y, 104) % 4 == 0)
                    {
                        r = 255;
                        g = 120;
                        b = 15;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 11:
                {
                    int r = 40 + (int)((ph(x,y,111) & 0x1F) - 15);
                    int g = 120 + (int)((ph(x,y,112) & 0x1F) - 15);
                    int b = 180 + (int)((ph(x,y,113) & 0x1F) - 15);
                    if (ph(x, y, 114) % 8 == 0)
                    {
                        r = 80;
                        g = 160;
                        b = 210;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 12:
                {
                    int r = 215 + (int)((ph(x, y, 121) & 0x1F) - 15);
                    int g = 185 + (int)((ph(x, y, 122) & 0x1F) - 15);
                    int b = 110 + (int)((ph(x, y, 123) & 0x17) - 12);
                    if (ph(x, y, 124) % 11 == 0)
                    {
                        r -= 22;
                        g -= 18;
                        b -= 10;
                    }

                    if (ph(x, y, 125) % 15 == 0)
                    {
                        r = std::min(255, r + 30);
                        g = std::min(255, g + 25);
                        b = std::min(255, b + 20);
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 13:
                {
                    int base = 145 + (int)((ph(x, y, 131) & 0x1F) - 15);
                    int r = base - 2;
                    int g = base - 5;
                    int b = base - 14;
                    if (ph(x, y, 132) % 5 == 0)
                    {
                        r = std::min(255, base + 55);
                        g = std::min(255, base + 52);
                        b = std::min(255, base + 40);
                    }
                    else if (ph(x, y, 133) % 7 == 0)
                    {
                        r = base - 45;
                        g = base - 48;
                        b = base - 50;
                    }
                    else if (ph(x, y, 134) % 11 == 0)
                    {
                        r = std::min(255, base + 28);
                        g = std::min(255, base + 25);
                        b = std::min(255, base + 18);
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 14:
                {
                    int base = 120 + (int)((ph(x, y, 141) & 0x27) - 20);
                    int r = base - 8;
                    int g = base - 4;
                    int b = base + 14;
                    if (ph(x, y, 14) % 7 == 0)
                    {
                        r = 22;
                        g = 22;
                        b = 24;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 15:
                {
                    int base = 120 + (int)((ph(x, y, 151) & 0x27) - 20);
                    int r = base - 8;
                    int g = base - 4;
                    int b = base + 14;
                    if (ph(x, y, 15) % 8 == 0)
                    {
                        r = 185;
                        g = 110;
                        b = 55;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 16:
                {
                    int base = 120 + (int)((ph(x, y, 161) & 0x27) - 20);
                    int r = base - 8;
                    int g = base - 4;
                    int b = base + 14;
                    if (ph(x, y, 16) % 9 == 0)
                    {
                        r = 230;
                        g = 195;
                        b = 30;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 17:
                {
                    float cx = (float)x - 7.5f;
                    float cy = (float)y - 7.5f;
                    float ax = std::abs(cx);
                    float ay = std::abs(cy);
                    bool onSeam = (std::abs(ax - ay) < 1.5f);
                    int n = (int)((ph(x, y, 175) & 0x0F) - 8);
                    if (onSeam)
                    {
                        int s = 148 + (int)((ph(x, y, 176) & 0x0F) - 8);
                        px = makePixel(s, s + 4, s + 8);
                    }
                    else if (ay > ax && cy < 0.f)
                    {
                        px = makePixel(std::clamp(182 + n, 0, 255),
                                       std::clamp(228 + n, 0, 255),
                                       std::clamp(198 + n, 0, 255));
                    }
                    else if (ay > ax)
                    {
                        px = makePixel(std::clamp(232 + n, 0, 255),
                                       std::clamp(215 + n, 0, 255),
                                       std::clamp(175 + n, 0, 255));
                    }
                    else if (ax > ay && cx < 0.f)
                    {
                        px = makePixel(std::clamp(180 + n, 0, 255),
                                       std::clamp(208 + n, 0, 255),
                                       std::clamp(238 + n, 0, 255));
                    }
                    else
                    {
                        px = makePixel(std::clamp(218 + n, 0, 255),
                                       std::clamp(192 + n, 0, 255),
                                       std::clamp(232 + n, 0, 255));
                    }

                    if (ax < 1.5f && ay < 1.5f)
                    {
                        px = makePixel(252, 254, 255);
                    }

                    break;
                }

                case 18:
                {
                    int r = 210 + (int)((ph(x, y, 181) & 0x1F) - 15);
                    int g = 190 + (int)((ph(x, y, 182) & 0x1F) - 15);
                    int b = 145 + (int)((ph(x, y, 183) & 0x17) - 12);
                    if (ph(x, y, 184) % 5 == 0)
                    {
                        r = r * 50 / 100;
                        g = g * 50 / 100;
                        b = b * 50 / 100;
                    }
                    else if (ph(x, y, 185) % 8 == 0)
                    {
                        r = std::min(255, r + 18);
                        g = std::min(255, g + 15);
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                default:
                    break;
            }
        }
    }
}

unsigned int TextureAtlas::applyBright(unsigned int px, float f)
{
    unsigned int a = (px >> 24) & 0xFF;
    unsigned int b = (px >> 16) & 0xFF;
    unsigned int g = (px >> 8) & 0xFF;
    unsigned int r = (px) & 0xFF;
    r = clampByte((int)(r * f));
    g = clampByte((int)(g * f));
    b = clampByte((int)(b * f));
    return (a << 24) | (b << 16) | (g << 8) | r;
}

void TextureAtlas::build()
{
    pixels.assign(WIDTH * HEIGHT, 0u);
    for (int t = 0; t < TILE_COUNT; t++)
    {
        paintTile(pixels.data(), t);
    }

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int TextureAtlas::buildIconAtlas(const int *topTiles, const int *sideTiles, int count) const
{
    const int ICON = TILE_SIZE;
    std::vector<unsigned int> buf(count * ICON * ICON, 0u);
    auto sampleTile = [&](int tile, int tx, int ty) -> unsigned int
    {
        if (tile < 0 || tile >= TILE_COUNT)
        {
            return 0u;
        }

        tx = std::clamp(tx, 0, TILE_SIZE - 1);
        ty = std::clamp(ty, 0, TILE_SIZE - 1);
        return pixels[(ty * WIDTH) + (tile * TILE_SIZE) + tx];
    };

    for (int i = 0; i < count; i++)
    {
        int top = topTiles[i];
        int side = sideTiles[i];
        for (int iy = 0; iy < ICON; iy++)
        {
            for (int ix = 0; ix < ICON; ix++)
            {
                unsigned int px = 0u;
                if (iy > 3)
                {
                    int ly = iy - 4;
                    if (ix < 8)
                    {
                        int a = ix * 2;
                        int b = (2 * ly) - ix;
                        if (a >= 0 && a <= TILE_SIZE && b >= 0 && b <= TILE_SIZE)
                        {
                            px = applyBright(sampleTile(side, a, b), 0.6f);
                        }
                    }
                    else
                    {
                        int a = (ix * 2) - TILE_SIZE;
                        int b = (2 * ly) - TILE_SIZE + ix;
                        if (a >= 0 && a <= TILE_SIZE && b >= 0 && b <= TILE_SIZE)
                        {
                            px = applyBright(sampleTile(side, a, b), 0.8f);
                        }
                    }
                }

                if (iy < 8)
                {
                    int a = (2 * iy) + ix - 8;
                    int b = (2 * iy) - ix + 8;
                    if (a >= 0 && a <= TILE_SIZE && b >= 0 && b <= TILE_SIZE)
                    {
                        px = sampleTile(top, a, b);
                    }
                }

                buf[(iy * count * ICON) + (i * ICON) + ix] = px;
            }
        }
    }

    unsigned int tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, count * ICON, ICON, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buf.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void TextureAtlas::bind(int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texId);
}

void TextureAtlas::uvRect(int tile, float &u0, float &v0, float &u1, float &v1)
{
    float tw = (float)TILE_SIZE / (float)WIDTH;
    u0 = tile * tw;
    u1 = u0 + tw;
    v0 = 0.f;
    v1 = 1.f;
}