#include "game/item/Potions.hpp"

#include "game/world/block/DefaultedMappedRegistry.hpp"

#include "game/item/Potion.hpp"
#include "game/world/block/SimpleRegistry.hpp"
#include "game/util/ResourceLocation.hpp"

struct ResourceLocationHash
{
    std::size_t operator()(const ResourceLocation& value) const { return value.hashCode(); }
};

struct ResourceLocationEqual
{
    bool operator()(const ResourceLocation& a, const ResourceLocation& b) const { return a == b; }
};

Potion* Potions::EMPTY = NULL;
Potion* Potions::WATER = NULL;
Potion* Potions::MUNDANE = NULL;
Potion* Potions::THICK = NULL;
Potion* Potions::AWKWARD = NULL;
Potion* Potions::NIGHT_VISION = NULL;
Potion* Potions::LONG_NIGHT_VISION = NULL;
Potion* Potions::INVISIBILITY = NULL;
Potion* Potions::LONG_INVISIBILITY = NULL;
Potion* Potions::LEAPING = NULL;
Potion* Potions::LONG_LEAPING = NULL;
Potion* Potions::STRONG_LEAPING = NULL;
Potion* Potions::FIRE_RESISTANCE = NULL;
Potion* Potions::LONG_FIRE_RESISTANCE = NULL;
Potion* Potions::SWIFTNESS = NULL;
Potion* Potions::LONG_SWIFTNESS = NULL;
Potion* Potions::STRONG_SWIFTNESS = NULL;
Potion* Potions::SLOWNESS = NULL;
Potion* Potions::LONG_SLOWNESS = NULL;
Potion* Potions::WATER_BREATHING = NULL;
Potion* Potions::LONG_WATER_BREATHING = NULL;
Potion* Potions::HEALING = NULL;
Potion* Potions::STRONG_HEALING = NULL;
Potion* Potions::HARMING = NULL;
Potion* Potions::STRONG_HARMING = NULL;
Potion* Potions::POISON = NULL;
Potion* Potions::LONG_POISON = NULL;
Potion* Potions::STRONG_POISON = NULL;
Potion* Potions::REGENERATION = NULL;
Potion* Potions::LONG_REGENERATION = NULL;
Potion* Potions::STRONG_REGENERATION = NULL;
Potion* Potions::STRENGTH = NULL;
Potion* Potions::LONG_STRENGTH = NULL;
Potion* Potions::STRONG_STRENGTH = NULL;
Potion* Potions::WEAKNESS = NULL;
Potion* Potions::LONG_WEAKNESS = NULL;
Potion* Potions::LUCK = NULL;
Potion* Potions::TURTLE_MASTER = NULL;
Potion* Potions::LONG_TURTLE_MASTER = NULL;
Potion* Potions::STRONG_TURTLE_MASTER = NULL;
Potion* Potions::SLOW_FALLING = NULL;
Potion* Potions::LONG_SLOW_FALLING = NULL;
Potion* Potions::WITHER = NULL;

Potion* Potions::get(std::wstring name)
{
    return Potion::potionRegistry->get(ResourceLocation(name));
}

void Potions::StaticCtor()
{
    EMPTY = get(L"empty");
    WATER = get(L"water");
    MUNDANE = get(L"mundane");
    THICK = get(L"thick");
    AWKWARD = get(L"awkward");
    NIGHT_VISION = get(L"night_vision");
    LONG_NIGHT_VISION = get(L"long_night_vision");
    INVISIBILITY = get(L"invisibility");
    LONG_INVISIBILITY = get(L"long_invisibility");
    LEAPING = get(L"leaping");
    LONG_LEAPING = get(L"long_leaping");
    STRONG_LEAPING = get(L"strong_leaping");
    FIRE_RESISTANCE = get(L"fire_resistance");
    LONG_FIRE_RESISTANCE = get(L"long_fire_resistance");
    SWIFTNESS = get(L"swiftness");
    LONG_SWIFTNESS = get(L"long_swiftness");
    STRONG_SWIFTNESS = get(L"strong_swiftness");
    SLOWNESS = get(L"slowness");
    LONG_SLOWNESS = get(L"long_slowness");
    WATER_BREATHING = get(L"water_breathing");
    LONG_WATER_BREATHING = get(L"long_water_breathing");
    HEALING = get(L"healing");
    STRONG_HEALING = get(L"strong_healing");
    HARMING = get(L"harming");
    STRONG_HARMING = get(L"strong_harming");
    POISON = get(L"poison");
    LONG_POISON = get(L"long_poison");
    STRONG_POISON = get(L"strong_poison");
    REGENERATION = get(L"regeneration");
    LONG_REGENERATION = get(L"long_regeneration");
    STRONG_REGENERATION = get(L"strong_regeneration");
    STRENGTH = get(L"strength");
    LONG_STRENGTH = get(L"long_strength");
    STRONG_STRENGTH = get(L"strong_strength");
    WEAKNESS = get(L"weakness");
    LONG_WEAKNESS = get(L"long_weakness");
    LUCK = get(L"luck");
    TURTLE_MASTER = get(L"turtle_master");
    LONG_TURTLE_MASTER = get(L"long_turtle_master");
    STRONG_TURTLE_MASTER = get(L"strong_turtle_master");
    WITHER = get(L"wither");
    SLOW_FALLING = get(L"slow_falling");
    LONG_SLOW_FALLING = get(L"long_slow_falling");
}
