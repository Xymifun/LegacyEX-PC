#ifndef GAME_WORLD_BLOCK_MATERIALCOLOR_HPP
#define GAME_WORLD_BLOCK_MATERIALCOLOR_HPP

#include "game/core/ColourTable.hpp"
#include "game/core/DyeColor.hpp"

class MaterialColor
{
public:
    static const MaterialColor* NONE;
    static const MaterialColor* GRASS;
    static const MaterialColor* SAND;
    static const MaterialColor* CLOTH;
    static const MaterialColor* FIRE;
    static const MaterialColor* ICE;
    static const MaterialColor* METAL;
    static const MaterialColor* PLANT;
    static const MaterialColor* SNOW;
    static const MaterialColor* CLAY;
    static const MaterialColor* DIRT;
    static const MaterialColor* STONE;
    static const MaterialColor* WATER;
    static const MaterialColor* WOOD;
    static const MaterialColor* QUARTZ;
    static const MaterialColor* COLOR_ORANGE;
    static const MaterialColor* COLOR_MAGENTA;
    static const MaterialColor* COLOR_LIGHT_BLUE;
    static const MaterialColor* COLOR_YELLOW;
    static const MaterialColor* COLOR_LIGHT_GREEN;
    static const MaterialColor* COLOR_PINK;
    static const MaterialColor* COLOR_GREY;
    static const MaterialColor* COLOR_SILVER;
    static const MaterialColor* COLOR_CYAN;
    static const MaterialColor* COLOR_PURPLE;
    static const MaterialColor* COLOR_BLUE;
    static const MaterialColor* COLOR_BROWN;
    static const MaterialColor* COLOR_GREEN;
    static const MaterialColor* COLOR_RED;
    static const MaterialColor* COLOR_BLACK;
    static const MaterialColor* GOLD;
    static const MaterialColor* DIAMOND;
    static const MaterialColor* LAPIS;
    static const MaterialColor* EMERALD;
    static const MaterialColor* PODZOL;
    static const MaterialColor* NETHER;
    static const MaterialColor* TERRACOTTA_WHITE;
    static const MaterialColor* TERRACOTTA_ORANGE;
    static const MaterialColor* TERRACOTTA_MAGENTA;
    static const MaterialColor* TERRACOTTA_LIGHT_BLUE;
    static const MaterialColor* TERRACOTTA_YELLOW;
    static const MaterialColor* TERRACOTTA_LIGHT_GREEN;
    static const MaterialColor* TERRACOTTA_PINK;
    static const MaterialColor* TERRACOTTA_GREY;
    static const MaterialColor* TERRACOTTA_SILVER;
    static const MaterialColor* TERRACOTTA_CYAN;
    static const MaterialColor* TERRACOTTA_PURPLE;
    static const MaterialColor* TERRACOTTA_BLUE;
    static const MaterialColor* TERRACOTTA_BROWN;
    static const MaterialColor* TERRACOTTA_GREEN;
    static const MaterialColor* TERRACOTTA_RED;
    static const MaterialColor* TERRACOTTA_BLACK;

    MaterialColor(int id, eMinecraftColour col);

    static void staticCtor();
    static void staticCtorAfterDyeColor();

    static const MaterialColor* getDyeMaterial(const DyeColor* dyeColor);

    unsigned int calculateRGBColor(int brightness) const;

    eMinecraftColour col;
    int id;

    static const MaterialColor** MATERIAL_COLORS;

private:
    static const MaterialColor** DYE_MATERIALS;
};

#endif
