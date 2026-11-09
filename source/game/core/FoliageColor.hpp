#ifndef GAME_CORE_FOLIAGECOLOR_HPP
#define GAME_CORE_FOLIAGECOLOR_HPP

#include "game/world/biome/arrayWithLength.hpp"

class FoliageColor
{
public:
    static void init(arrayWithLength<int> newPixels);

    static unsigned int get(double temperature, double downfall);
    static void set(double temperature, double downfall, unsigned int colour);

    static unsigned int getEvergreenColor();
    static unsigned int getBirchColor();
    static unsigned int getDefaultColor();

private:
    static arrayWithLength<int> pixels;
};

#endif
