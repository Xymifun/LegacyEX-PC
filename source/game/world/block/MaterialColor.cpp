#include "game/world/block/MaterialColor.hpp"

#include "game/core/DyeColor.hpp"

const MaterialColor* MaterialColor::NONE;
const MaterialColor* MaterialColor::GRASS;
const MaterialColor* MaterialColor::SAND;
const MaterialColor* MaterialColor::CLOTH;
const MaterialColor* MaterialColor::FIRE;
const MaterialColor* MaterialColor::ICE;
const MaterialColor* MaterialColor::METAL;
const MaterialColor* MaterialColor::PLANT;
const MaterialColor* MaterialColor::SNOW;
const MaterialColor* MaterialColor::CLAY;
const MaterialColor* MaterialColor::DIRT;
const MaterialColor* MaterialColor::STONE;
const MaterialColor* MaterialColor::WATER;
const MaterialColor* MaterialColor::WOOD;
const MaterialColor* MaterialColor::QUARTZ;
const MaterialColor* MaterialColor::COLOR_ORANGE;
const MaterialColor* MaterialColor::COLOR_MAGENTA;
const MaterialColor* MaterialColor::COLOR_LIGHT_BLUE;
const MaterialColor* MaterialColor::COLOR_YELLOW;
const MaterialColor* MaterialColor::COLOR_LIGHT_GREEN;
const MaterialColor* MaterialColor::COLOR_PINK;
const MaterialColor* MaterialColor::COLOR_GREY;
const MaterialColor* MaterialColor::COLOR_SILVER;
const MaterialColor* MaterialColor::COLOR_CYAN;
const MaterialColor* MaterialColor::COLOR_PURPLE;
const MaterialColor* MaterialColor::COLOR_BLUE;
const MaterialColor* MaterialColor::COLOR_BROWN;
const MaterialColor* MaterialColor::COLOR_GREEN;
const MaterialColor* MaterialColor::COLOR_RED;
const MaterialColor* MaterialColor::COLOR_BLACK;
const MaterialColor* MaterialColor::GOLD;
const MaterialColor* MaterialColor::DIAMOND;
const MaterialColor* MaterialColor::LAPIS;
const MaterialColor* MaterialColor::EMERALD;
const MaterialColor* MaterialColor::PODZOL;
const MaterialColor* MaterialColor::NETHER;
const MaterialColor* MaterialColor::TERRACOTTA_WHITE;
const MaterialColor* MaterialColor::TERRACOTTA_ORANGE;
const MaterialColor* MaterialColor::TERRACOTTA_MAGENTA;
const MaterialColor* MaterialColor::TERRACOTTA_LIGHT_BLUE;
const MaterialColor* MaterialColor::TERRACOTTA_YELLOW;
const MaterialColor* MaterialColor::TERRACOTTA_LIGHT_GREEN;
const MaterialColor* MaterialColor::TERRACOTTA_PINK;
const MaterialColor* MaterialColor::TERRACOTTA_GREY;
const MaterialColor* MaterialColor::TERRACOTTA_SILVER;
const MaterialColor* MaterialColor::TERRACOTTA_CYAN;
const MaterialColor* MaterialColor::TERRACOTTA_PURPLE;
const MaterialColor* MaterialColor::TERRACOTTA_BLUE;
const MaterialColor* MaterialColor::TERRACOTTA_BROWN;
const MaterialColor* MaterialColor::TERRACOTTA_GREEN;
const MaterialColor* MaterialColor::TERRACOTTA_RED;
const MaterialColor* MaterialColor::TERRACOTTA_BLACK;

const MaterialColor** MaterialColor::MATERIAL_COLORS;
const MaterialColor** MaterialColor::DYE_MATERIALS;

MaterialColor::MaterialColor(int id, eMinecraftColour col)
{
    this->id = id;
    this->col = col;

    MATERIAL_COLORS[id] = this;
}

void MaterialColor::staticCtor()
{
    MATERIAL_COLORS = new const MaterialColor*[64];

    NONE = new MaterialColor(0, eMinecraftColour_Material_None);
    GRASS = new MaterialColor(1, eMinecraftColour_Material_Grass);
    SAND = new MaterialColor(2, eMinecraftColour_Material_Sand);
    CLOTH = new MaterialColor(3, eMinecraftColour_Material_Cloth);
    FIRE = new MaterialColor(4, eMinecraftColour_Material_Fire);
    ICE = new MaterialColor(5, eMinecraftColour_Material_Ice);
    METAL = new MaterialColor(6, eMinecraftColour_Material_Metal);
    PLANT = new MaterialColor(7, eMinecraftColour_Material_Plant);
    SNOW = new MaterialColor(8, eMinecraftColour_Material_Snow);
    CLAY = new MaterialColor(9, eMinecraftColour_Material_Clay);
    DIRT = new MaterialColor(10, eMinecraftColour_Material_Dirt);
    STONE = new MaterialColor(11, eMinecraftColour_Material_Stone);
    WATER = new MaterialColor(12, eMinecraftColour_Material_Water);
    WOOD = new MaterialColor(13, eMinecraftColour_Material_Wood);
    QUARTZ = new MaterialColor(14, eMinecraftColour_Material_Quartz);
    COLOR_ORANGE = new MaterialColor(15, eMinecraftColour_Material_Color_Orange);
    COLOR_MAGENTA = new MaterialColor(16, eMinecraftColour_Material_Color_Magenta);
    COLOR_LIGHT_BLUE = new MaterialColor(17, eMinecraftColour_Material_Color_Light_Blue);
    COLOR_YELLOW = new MaterialColor(18, eMinecraftColour_Material_Color_Yellow);
    COLOR_LIGHT_GREEN = new MaterialColor(19, eMinecraftColour_Material_Color_Light_Green);
    COLOR_PINK = new MaterialColor(20, eMinecraftColour_Material_Color_Pink);
    COLOR_GREY = new MaterialColor(21, eMinecraftColour_Material_Color_Grey);
    COLOR_SILVER = new MaterialColor(22, eMinecraftColour_Material_Color_Silver);
    COLOR_CYAN = new MaterialColor(23, eMinecraftColour_Material_Color_Cyan);
    COLOR_PURPLE = new MaterialColor(24, eMinecraftColour_Material_Color_Purple);
    COLOR_BLUE = new MaterialColor(25, eMinecraftColour_Material_Color_Blue);
    COLOR_BROWN = new MaterialColor(26, eMinecraftColour_Material_Color_Brown);
    COLOR_GREEN = new MaterialColor(27, eMinecraftColour_Material_Color_Green);
    COLOR_RED = new MaterialColor(28, eMinecraftColour_Material_Color_Red);
    COLOR_BLACK = new MaterialColor(29, eMinecraftColour_Material_Color_Black);
    GOLD = new MaterialColor(30, eMinecraftColour_Material_Gold);
    DIAMOND = new MaterialColor(31, eMinecraftColour_Material_Diamond);
    LAPIS = new MaterialColor(32, eMinecraftColour_Material_Lapis);
    EMERALD = new MaterialColor(33, eMinecraftColour_Material_Emerald);
    PODZOL = new MaterialColor(34, eMinecraftColour_Material_Podzol);
    NETHER = new MaterialColor(35, eMinecraftColour_Material_Nether);
    TERRACOTTA_WHITE = new MaterialColor(36, eMinecraftColour_Material_Terracotta_White);
    TERRACOTTA_ORANGE = new MaterialColor(37, eMinecraftColour_Material_Terracotta_Orange);
    TERRACOTTA_MAGENTA = new MaterialColor(38, eMinecraftColour_Material_Terracotta_Magenta);
    TERRACOTTA_LIGHT_BLUE = new MaterialColor(39, eMinecraftColour_Material_Terracotta_Light_Blue);
    TERRACOTTA_YELLOW = new MaterialColor(40, eMinecraftColour_Material_Terracotta_Yellow);
    TERRACOTTA_LIGHT_GREEN = new MaterialColor(41, eMinecraftColour_Material_Terracotta_Light_Green);
    TERRACOTTA_PINK = new MaterialColor(42, eMinecraftColour_Material_Terracotta_Pink);
    TERRACOTTA_GREY = new MaterialColor(43, eMinecraftColour_Material_Terracotta_Grey);
    TERRACOTTA_SILVER = new MaterialColor(44, eMinecraftColour_Material_Terracotta_Silver);
    TERRACOTTA_CYAN = new MaterialColor(45, eMinecraftColour_Material_Terracotta_Cyan);
    TERRACOTTA_PURPLE = new MaterialColor(46, eMinecraftColour_Material_Terracotta_Purple);
    TERRACOTTA_BLUE = new MaterialColor(47, eMinecraftColour_Material_Terracotta_Blue);
    TERRACOTTA_BROWN = new MaterialColor(48, eMinecraftColour_Material_Terracotta_Brown);
    TERRACOTTA_GREEN = new MaterialColor(49, eMinecraftColour_Material_Terracotta_Green);
    TERRACOTTA_RED = new MaterialColor(50, eMinecraftColour_Material_Terracotta_Red);
    TERRACOTTA_BLACK = new MaterialColor(51, eMinecraftColour_Material_Terracotta_Black);
}

void MaterialColor::staticCtorAfterDyeColor()
{
    DYE_MATERIALS = new const MaterialColor*[16];

    DYE_MATERIALS[DyeColor::WHITE->getBlockData()] = SNOW;
    DYE_MATERIALS[DyeColor::ORANGE->getBlockData()] = COLOR_ORANGE;
    DYE_MATERIALS[DyeColor::MAGENTA->getBlockData()] = COLOR_MAGENTA;
    DYE_MATERIALS[DyeColor::LIGHT_BLUE->getBlockData()] = COLOR_LIGHT_BLUE;
    DYE_MATERIALS[DyeColor::YELLOW->getBlockData()] = COLOR_YELLOW;
    DYE_MATERIALS[DyeColor::LIME->getBlockData()] = COLOR_LIGHT_GREEN;
    DYE_MATERIALS[DyeColor::PINK->getBlockData()] = COLOR_PINK;
    DYE_MATERIALS[DyeColor::GRAY->getBlockData()] = COLOR_GREY;
    DYE_MATERIALS[DyeColor::SILVER->getBlockData()] = COLOR_SILVER;
    DYE_MATERIALS[DyeColor::CYAN->getBlockData()] = COLOR_CYAN;
    DYE_MATERIALS[DyeColor::PURPLE->getBlockData()] = COLOR_PURPLE;
    DYE_MATERIALS[DyeColor::BLUE->getBlockData()] = COLOR_BLUE;
    DYE_MATERIALS[DyeColor::BROWN->getBlockData()] = COLOR_BROWN;
    DYE_MATERIALS[DyeColor::GREEN->getBlockData()] = COLOR_GREEN;
    DYE_MATERIALS[DyeColor::RED->getBlockData()] = COLOR_RED;
    DYE_MATERIALS[DyeColor::BLACK->getBlockData()] = COLOR_BLACK;
}

unsigned int MaterialColor::calculateRGBColor(int brightness) const
{
    int scale = 220;

    if (brightness == 3)
    {
        scale = 135;
    }

    if (brightness == 2)
    {
        scale = 255;
    }

    if (brightness == 1)
    {
        scale = 220;
    }

    if (brightness == 0)
    {
        scale = 180;
    }

    int red = ((col >> 16) & 0xFF) * scale / 255;
    int green = ((col >> 8) & 0xFF) * scale / 255;
    int blue = (col & 0xFF) * scale / 255;

    return 0xFF000000 | (red << 16) | (green << 8) | blue;
}

const MaterialColor* MaterialColor::getDyeMaterial(const DyeColor* dyeColor)
{
    return DYE_MATERIALS[dyeColor->getBlockData()];
}
