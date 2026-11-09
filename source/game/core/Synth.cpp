#include "game/core/Synth.hpp"

arrayWithLength<double> Synth::create(int width, int height)
{
    arrayWithLength<double> values(width * height, true);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            values.data[(unsigned int)(x + y * width)] = getValue((double)x, (double)y);
        }
    }

    return values;
}
