#ifndef GAME_ITEM_ENCHANTMENTS_HPP
#define GAME_ITEM_ENCHANTMENTS_HPP

#include <string>

class Enchantment;

class Enchantments
{
public:
    static void staticCtor();

    static Enchantment* ALL_DAMAGE_PROTECTION;
    static Enchantment* FIRE_PROTECTION;
    static Enchantment* FALL_PROTECTION;
    static Enchantment* BLAST_PROTECTION;
    static Enchantment* PROJECTILE_PROTECTION;
    static Enchantment* RESPIRATION;
    static Enchantment* AQUA_AFFINITY;
    static Enchantment* THORNS;
    static Enchantment* DEPTH_STRIDER;
    static Enchantment* FROST_WALKER;
    static Enchantment* BINDING_CURSE;
    static Enchantment* SHARPNESS;
    static Enchantment* SMITE;
    static Enchantment* BANE_OF_ARTHROPODS;
    static Enchantment* KNOCKBACK;
    static Enchantment* FIRE_ASPECT;
    static Enchantment* MOB_LOOTING;
    static Enchantment* BLOCK_EFFICIENCY;
    static Enchantment* SILK_TOUCH;
    static Enchantment* UNBREAKING;
    static Enchantment* BLOCK_FORTUNE;
    static Enchantment* POWER_ARROWS;
    static Enchantment* PUNCH_ARROWS;
    static Enchantment* FLAMING_ARROWS;
    static Enchantment* INFINITY_ARROWS;
    static Enchantment* FISHING_LUCK;
    static Enchantment* FISHING_SPEED;
    static Enchantment* MENDING;
    static Enchantment* VANISHING_CURSE;
    static Enchantment* IMPALING;
    static Enchantment* RIPTIDE;
    static Enchantment* LOYALTY;
    static Enchantment* CHANNELING;

private:
    static Enchantment* get(const std::wstring& name);
};

#endif
