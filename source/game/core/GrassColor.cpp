#include "game/core/GrassColor.hpp"

arrayWithLength<int> GrassColor::pixels;

void GrassColor::init(arrayWithLength<int> newPixels)
{
    int* oldPixels = pixels.data;

    pixels = newPixels;

    delete[] oldPixels;
}

unsigned int GrassColor::get(double temperature, double downfall)
{
    int x = (int)((1.0 - temperature) * 255.0);
    int y = (int)((1.0 - downfall * temperature) * 255.0);

    unsigned int index = (unsigned int)((y << 8) | x);

    unsigned int colour = 0xffff00ff;

    if (index <= pixels.length)
    {
        colour = pixels.data[index];
    }

    return colour;
}

void GrassColor::set(double temperature, double downfall, unsigned int colour)
{
    int x = (int)((1.0 - temperature) * 255.0);
    int y = (int)((1.0 - downfall * temperature) * 255.0);

    unsigned int index = (unsigned int)((y << 8) | x);

    if (index <= pixels.length)
    {
        pixels.data[index] = colour;
    }
}
