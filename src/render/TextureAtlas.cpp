#include "TextureAtlas.hpp"
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
                    int r = 168 + (int)((ph(x, y, 61) & 0x0F) - 8);
                    int g = 112 + (int)((ph(x, y, 62) & 0x0F) - 8);
                    int b = 48 + (int)((ph(x, y, 63) & 0x0F) - 8);
                    int band = (y + (int)(ph(x, 0, 64) & 0x03)) % 4;
                    if (band == 0)
                    {
                        r = r * 7 / 10;
                        g = g * 7 / 10;
                        b = b * 7 / 10;
                    }
                    else if (band == 3)
                    {
                        r = std::min(255, r + 12);
                        g = std::min(255, g + 8);
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 7:
                {
                    bool onStem = (x >= 7 && x <= 8);
                    bool onLeafL = (y >= 4 && y <= 6) && (x >= 3 && x <= 6);
                    bool onLeafR = (y >= 4 && y <= 6) && (x >= 9 && x <= 12);
                    if (!onStem && !onLeafL && !onLeafR)
                    {
                        px = 0x00000000u;
                    }
                    else
                    {
                        int r = 45 + (int)((ph(x, y, 71) & 0x1F) - 15);
                        int g = 130 + (int)((ph(x, y, 72) & 0x1F) - 15);
                        int b = 25 + (int)((ph(x, y, 73) & 0x1F) - 15);
                        if (ph(x, y, 74) % 7 == 0)
                        {
                            r = 25;
                            g = 165;
                            b = 40;
                        }

                        px = makePixel(r, g, b) | 0xFF000000u;
                    }

                    break;
                }

                case 8:
                {
                    int r = 200 + (int)((ph(x, y, 81) & 0x0F) - 8);
                    int g = 165 + (int)((ph(x, y, 82) & 0x0F) - 8);
                    int b = 95 + (int)((ph(x, y, 83) & 0x0F) - 8);
                    int row = y % 4;
                    if (row == 0)
                    {
                        r = r * 6 / 10;
                        g = g * 6 / 10;
                        b = b * 6 / 10;
                    }
                    else if (row == 3)
                    {
                        r = std::min(255, r + 18);
                        g = std::min(255, g + 14);
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
                    int r = 220 + (int)((ph(x, y, 101) & 0x1F) - 20);
                    int g = 50 + (int)((ph(x, y, 102) & 0x1F) - 20);
                    int b = 10 + (int)((ph(x, y, 103) & 0x0F) - 8);
                    if (ph(x, y, 104) % 6 == 0)
                    {
                        r = 255;
                        g = 100;
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
                    int r = 195 + (int)((ph(x, y, 121) & 0x1F) - 15);
                    int g = 168 + (int)((ph(x, y, 122) & 0x1F) - 15);
                    int b = 105 + (int)((ph(x, y, 123) & 0x17) - 12);
                    if (ph(x, y, 124) % 11 == 0)
                    {
                        r -= 18;
                        g -= 14;
                        b -= 8;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 13:
                {
                    int base = 130 + (int)((ph(x, y, 131) & 0x1F) - 15);
                    int r = base - 4;
                    int g = base - 8;
                    int b = base - 18;
                    if (ph(x, y, 132) % 6 == 0)
                    {
                        r -= 30;
                        g -= 28;
                        b -= 25;
                    }
                    else if (ph(x, y, 133) % 9 == 0)
                    {
                        r += 22;
                        g += 20;
                        b += 16;
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

                default:
                    px = 0xFF808080u;
            }
        }
    }
}

void TextureAtlas::build()
{
    std::vector<unsigned int> pixels(WIDTH * HEIGHT, 0);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
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