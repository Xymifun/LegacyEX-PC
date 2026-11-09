#ifndef GAME_CORE_GRASSCOLOR_HPP
#define GAME_CORE_GRASSCOLOR_HPP

#include "game/world/biome/arrayWithLength.hpp"

class GrassColor
{
public:
    static void init(arrayWithLength<int> newPixels);

    static unsigned int get(double temperature, double downfall);
    static void set(double temperature, double downfall, unsigned int colour);

private:
    static arrayWithLength<int> pixels;
};

#endif
