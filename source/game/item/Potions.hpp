#ifndef GAME_ITEM_POTIONS_HPP
#define GAME_ITEM_POTIONS_HPP

#include <string>

class Potion;

class Potions
{
public:
    static void StaticCtor();

    static Potion* EMPTY;
    static Potion* WATER;
    static Potion* MUNDANE;
    static Potion* THICK;
    static Potion* AWKWARD;
    static Potion* NIGHT_VISION;
    static Potion* LONG_NIGHT_VISION;
    static Potion* INVISIBILITY;
    static Potion* LONG_INVISIBILITY;
    static Potion* LEAPING;
    static Potion* LONG_LEAPING;
    static Potion* STRONG_LEAPING;
    static Potion* FIRE_RESISTANCE;
    static Potion* LONG_FIRE_RESISTANCE;
    static Potion* SWIFTNESS;
    static Potion* LONG_SWIFTNESS;
    static Potion* STRONG_SWIFTNESS;
    static Potion* SLOWNESS;
    static Potion* LONG_SLOWNESS;
    static Potion* WATER_BREATHING;
    static Potion* LONG_WATER_BREATHING;
    static Potion* HEALING;
    static Potion* STRONG_HEALING;
    static Potion* HARMING;
    static Potion* STRONG_HARMING;
    static Potion* POISON;
    static Potion* LONG_POISON;
    static Potion* STRONG_POISON;
    static Potion* REGENERATION;
    static Potion* LONG_REGENERATION;
    static Potion* STRONG_REGENERATION;
    static Potion* STRENGTH;
    static Potion* LONG_STRENGTH;
    static Potion* STRONG_STRENGTH;
    static Potion* WEAKNESS;
    static Potion* LONG_WEAKNESS;
    static Potion* LUCK;
    static Potion* TURTLE_MASTER;
    static Potion* LONG_TURTLE_MASTER;
    static Potion* STRONG_TURTLE_MASTER;
    static Potion* SLOW_FALLING;
    static Potion* LONG_SLOW_FALLING;
    static Potion* WITHER;

private:
    static Potion* get(std::wstring name);
};

#endif
