#include "game/core/FoliageColor.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"

arrayWithLength<int> FoliageColor::pixels;

void FoliageColor::init(arrayWithLength<int> newPixels)
{
    int* oldPixels = pixels.data;

    pixels = newPixels;

    delete[] oldPixels;
}

unsigned int FoliageColor::get(double temperature, double downfall)
{
    int x = (int)((1.0 - temperature) * 255.0);
    int y = (int)((1.0 - downfall * temperature) * 255.0);

    return pixels.data[(y << 8) | x];
}

void FoliageColor::set(double temperature, double downfall, unsigned int colour)
{
    int x = (int)((1.0 - temperature) * 255.0);
    int y = (int)((1.0 - downfall * temperature) * 255.0);

    pixels.data[(y << 8) | x] = colour;
}

unsigned int FoliageColor::getEvergreenColor()
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Foliage_Evergreen);
}

unsigned int FoliageColor::getBirchColor()
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Foliage_Birch);
}

unsigned int FoliageColor::getDefaultColor()
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Foliage_Default);
}
