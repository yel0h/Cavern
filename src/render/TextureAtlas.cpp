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
                    int r = 76 + (int)((ph(x, y, 1) & 0x1F) - 15);
                    int g = 128 + (int)((ph(x, y, 11) & 0x1F) - 15);
                    int b = 48 + (int)((ph(x, y, 111) & 0x1F) - 15);
                    if (ph(x, y, 2) % 25 == 0)
                    {
                        r = 160;
                        g = 210;
                        b = 90;
                    }

                    px = makePixel(r, g, b);
                    break;
                }

                case 2:
                {
                    if (y < 4)
                    {
                        int r = 76 + (int)((ph(x, y, 1) & 0x1F) - 15);
                        int g = 128 + (int)((ph(x, y, 11) & 0x1F) - 15);
                        int b = 48 + (int)((ph(x, y, 111) & 0x1F) - 15);
                        if (ph(x, y, 2) % 25 == 0)
                        {
                            r = 160;
                            g = 210;
                            b = 90;
                        }

                        px = makePixel(r, g, b);
                    }
                    else
                    {
                        int r = 120 + (int)((ph(x, y, 22) & 0x17) - 12);
                        int g = 72 + (int)((ph(x, y, 222) & 0x17) - 12);
                        int b = 24 + (int)((ph(x, y, 2222) & 0x17) - 12);
                        px = makePixel(r, g, b);
                    }

                    break;
                }

                case 3:
                {
                    int r = 120 + (int)((ph(x, y, 33) & 0x17) - 12);
                    int g = 72 + (int)((ph(x, y, 333) & 0x17) - 12);
                    int b = 24 + (int)((ph(x, y, 3333) & 0x17) - 12);
                    px = makePixel(r, g, b);
                    break;
                }

                case 4:
                {
                    int base = 128 + (int)((h & 0x27) - 20);
                    int r = base;
                    int g = base;
                    int b = base;
                    if (ph(x, y, 5) % 12 == 0)
                    {
                        unsigned int fleck = ph(x, y, 55);
                        int ch = fleck % 3;
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