#include "game/item/Enchantments.hpp"

#include "game/item/Enchantment.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/util/ResourceLocation.hpp"

Enchantment* Enchantments::ALL_DAMAGE_PROTECTION = NULL;
Enchantment* Enchantments::FIRE_PROTECTION = NULL;
Enchantment* Enchantments::FALL_PROTECTION = NULL;
Enchantment* Enchantments::BLAST_PROTECTION = NULL;
Enchantment* Enchantments::PROJECTILE_PROTECTION = NULL;
Enchantment* Enchantments::RESPIRATION = NULL;
Enchantment* Enchantments::AQUA_AFFINITY = NULL;
Enchantment* Enchantments::THORNS = NULL;
Enchantment* Enchantments::DEPTH_STRIDER = NULL;
Enchantment* Enchantments::FROST_WALKER = NULL;
Enchantment* Enchantments::BINDING_CURSE = NULL;
Enchantment* Enchantments::SHARPNESS = NULL;
Enchantment* Enchantments::SMITE = NULL;
Enchantment* Enchantments::BANE_OF_ARTHROPODS = NULL;
Enchantment* Enchantments::KNOCKBACK = NULL;
Enchantment* Enchantments::FIRE_ASPECT = NULL;
Enchantment* Enchantments::MOB_LOOTING = NULL;
Enchantment* Enchantments::BLOCK_EFFICIENCY = NULL;
Enchantment* Enchantments::SILK_TOUCH = NULL;
Enchantment* Enchantments::UNBREAKING = NULL;
Enchantment* Enchantments::BLOCK_FORTUNE = NULL;
Enchantment* Enchantments::POWER_ARROWS = NULL;
Enchantment* Enchantments::PUNCH_ARROWS = NULL;
Enchantment* Enchantments::FLAMING_ARROWS = NULL;
Enchantment* Enchantments::INFINITY_ARROWS = NULL;
Enchantment* Enchantments::FISHING_LUCK = NULL;
Enchantment* Enchantments::FISHING_SPEED = NULL;
Enchantment* Enchantments::MENDING = NULL;
Enchantment* Enchantments::VANISHING_CURSE = NULL;
Enchantment* Enchantments::IMPALING = NULL;
Enchantment* Enchantments::RIPTIDE = NULL;
Enchantment* Enchantments::LOYALTY = NULL;
Enchantment* Enchantments::CHANNELING = NULL;

Enchantment* Enchantments::get(const std::wstring& name)
{
    return Enchantment::enchantmentRegistry->get(ResourceLocation(name));
}

void Enchantments::staticCtor()
{
    ALL_DAMAGE_PROTECTION = get(L"protection");
    FIRE_PROTECTION = get(L"fire_protection");
    FALL_PROTECTION = get(L"feather_falling");
    BLAST_PROTECTION = get(L"blast_protection");
    PROJECTILE_PROTECTION = get(L"projectile_protection");
    RESPIRATION = get(L"respiration");
    AQUA_AFFINITY = get(L"aqua_affinity");
    THORNS = get(L"thorns");
    DEPTH_STRIDER = get(L"depth_strider");
    FROST_WALKER = get(L"frost_walker");
    BINDING_CURSE = get(L"binding_curse");
    SHARPNESS = get(L"sharpness");
    SMITE = get(L"smite");
    BANE_OF_ARTHROPODS = get(L"bane_of_arthropods");
    KNOCKBACK = get(L"knockback");
    FIRE_ASPECT = get(L"fire_aspect");
    MOB_LOOTING = get(L"looting");
    BLOCK_EFFICIENCY = get(L"efficiency");
    SILK_TOUCH = get(L"silk_touch");
    UNBREAKING = get(L"unbreaking");
    BLOCK_FORTUNE = get(L"fortune");
    POWER_ARROWS = get(L"power");
    PUNCH_ARROWS = get(L"punch");
    FLAMING_ARROWS = get(L"flame");
    INFINITY_ARROWS = get(L"infinity");
    FISHING_LUCK = get(L"luck_of_the_sea");
    FISHING_SPEED = get(L"lure");
    MENDING = get(L"mending");
    VANISHING_CURSE = get(L"vanishing_curse");
    IMPALING = get(L"impaling");
    RIPTIDE = get(L"riptide");
    LOYALTY = get(L"loyalty");
    CHANNELING = get(L"channeling");
}
