#include "game/client/render/MobSkinMemTextureProcessor.hpp"
#include "game/core/BufferedImage.hpp"

MobSkinMemTextureProcessor::~MobSkinMemTextureProcessor()
{
}

void MobSkinMemTextureProcessor::setNoAlpha(int x0, int y0, int x1, int y1)
{
    int x;
    for (x = x0; x < x1; ++x)
    {
        int y;
        for (y = y0; y < y1; ++y)
            m_data[x + y * m_width] |= (int)0xff000000u;
    }
}

bool MobSkinMemTextureProcessor::hasAlpha(int x0, int y0, int x1, int y1)
{
    int x;
    for (x = x0; x < x1; ++x)
    {
        int y;
        for (y = y0; y < y1; ++y)
        {
            int pixel = m_data[x + y * m_width];
            if (((pixel >> 24) & 0xff) < 128)
                return true;
        }
    }
    return false;
}

void MobSkinMemTextureProcessor::setForceAlpha(int x0, int y0, int x1, int y1)
{
    if (!hasAlpha(x0, y0, x1, y1))
    {
        int x;
        for (x = x0; x < x1; ++x)
        {
            int y;
            for (y = y0; y < y1; ++y)
                m_data[x + y * m_width] &= 0x00ffffff;
        }
    }
}

BufferedImage* MobSkinMemTextureProcessor::process(BufferedImage* image)
{
    if (image == NULL)
        return NULL;
    m_width = 0x40;
    m_height = 0x20;
    BufferedImage* result = new BufferedImage(m_width, m_height, 0);
    result->getGraphics();
    m_data = result->getData();
    setNoAlpha(0, 0, 0x20, 0x10);
    setForceAlpha(0x20, 0, 0x40, 0x20);
    setNoAlpha(0, 0x10, 0x40, 0x20);
    bool transparent = false;
    int x;
    for (x = 32; x < 64; ++x)
    {
        int y;
        for (y = 0; y < 16; ++y)
        {
            int pixel = m_data[x + y * 64];
            if (((pixel >> 24) & 0xff) < 128)
                transparent = true;
        }
    }
    if (!transparent)
    {
        for (x = 32; x < 64; ++x)
        {
            int y;
            for (y = 0; y < 16; ++y)
            {
                int pixel = m_data[x + y * 64];
                pixel &= 0x00ffffff;
            }
        }
    }
    return result;
}
