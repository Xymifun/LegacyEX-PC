#include "game/item/Potion.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/L10N.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Items.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"

static ResourceLocation* DEFAULT_POTION_KEY = new ResourceLocation(L"water");
DefaultedMappedRegistry<ResourceLocation, Potion*>* Potion::potionRegistry =
    new DefaultedMappedRegistry<ResourceLocation, Potion*>(*DEFAULT_POTION_KEY);
static int gNextPotionId = 0;

Potion* Potion::byId(int id)
{
    return potionRegistry->byId(id);
}

int Potion::getId(Potion* potion)
{
    return potionRegistry->getId(potion);
}

Potion* Potion::byName(const std::wstring& name)
{
    return potionRegistry->get(ResourceLocation(name));
}

Potion::Potion(int nameId, int useDescriptionId, int splashNameId, int lingeringNameId, int arrowNameId,
               int arrowUseDescriptionId)
{
    mNameId = nameId;
    mSplashNameId = splashNameId;
    mLingeringNameId = lingeringNameId;
    mArrowNameId = arrowNameId;
    mUseDescriptionId = useDescriptionId;
    mArrowUseDescriptionId = arrowUseDescriptionId;
}

Potion::Potion(MobEffectInstance* effect, int nameId, int useDescriptionId, int splashNameId, int lingeringNameId,
               int arrowNameId, int arrowUseDescriptionId)
{
    mNameId = nameId;
    mSplashNameId = splashNameId;
    mLingeringNameId = lingeringNameId;
    mArrowNameId = arrowNameId;
    mUseDescriptionId = useDescriptionId;
    mArrowUseDescriptionId = arrowUseDescriptionId;
    mEffects.push_back(effect);
}

Potion::Potion(const std::vector<MobEffectInstance*>& effects, int nameId, int useDescriptionId, int splashNameId,
               int lingeringNameId, int arrowNameId, int arrowUseDescriptionId)
    : mNameId(nameId),
      mSplashNameId(splashNameId),
      mLingeringNameId(lingeringNameId),
      mArrowNameId(arrowNameId),
      mUseDescriptionId(useDescriptionId),
      mArrowUseDescriptionId(arrowUseDescriptionId),
      mEffects(effects)
{
}

std::wstring Potion::getName(EType type)
{
    int id;
    switch (type)
    {
    case NORMAL:
        id = mNameId;
        break;
    case SPLASH:
        id = mSplashNameId;
        break;
    case LINGERING:
        id = mLingeringNameId;
        break;
    case ARROW:
        id = mArrowNameId;
        break;
    default:
        id = mNameId;
        break;
    }
    return L10N::GetString(id);
}

std::vector<MobEffectInstance*>& Potion::getEffects()
{
    return mEffects;
}

int Potion::getUseDescriptionId(EType type)
{
    return type == ARROW ? mArrowUseDescriptionId : mUseDescriptionId;
}

void Potion::addPotion(const std::wstring& name, Potion* potion)
{
    potionRegistry->registerMapping(gNextPotionId++, ResourceLocation(name), potion);
}

inline MobEffectInstance* Potion::effectWithId(MobEffectInstance* effect, int id)
{
    effect->durationEffectId = id;
    return effect;
}

void Potion::StaticCtor()
{
    MobEffectInstance* effect;
    addPotion(L"empty", new Potion(IDS_POTION_EFFECT_EMPTY, IDS_POTION_DESC_EMPTY, IDS_SPLASH_POTION_EFFECT_EMPTY,
                                   IDS_LINGERING_POTION_EFFECT_EMPTY, IDS_TIPPED_ARROW_EFFECT_EMPTY,
                                   IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"water", new Potion(IDS_POTION_EFFECT_WATER, IDS_POTION_DESC_WATER_BOTTLE,
                                   IDS_SPLASH_POTION_EFFECT_WATER, IDS_LINGERING_POTION_EFFECT_WATER,
                                   IDS_TIPPED_ARROW_EFFECT_WATER, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"mundane", new Potion(IDS_POTION_EFFECT_MUNDANE, IDS_POTION_DESC_WATER_BOTTLE,
                                     IDS_SPLASH_POTION_EFFECT_MUNDANE, IDS_LINGERING_POTION_EFFECT_MUNDANE,
                                     IDS_TIPPED_ARROW_EFFECT_MUNDANE, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"thick", new Potion(IDS_POTION_EFFECT_THICK, IDS_POTION_DESC_WATER_BOTTLE,
                                   IDS_SPLASH_POTION_EFFECT_THICK, IDS_LINGERING_POTION_EFFECT_THICK,
                                   IDS_TIPPED_ARROW_EFFECT_THICK, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"awkward", new Potion(IDS_POTION_EFFECT_AWKWARD, IDS_POTION_DESC_WATER_BOTTLE,
                                     IDS_SPLASH_POTION_EFFECT_AWKWARD, IDS_LINGERING_POTION_EFFECT_AWKWARD,
                                     IDS_TIPPED_ARROW_EFFECT_AWKWARD, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));

    addPotion(L"night_vision",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::NIGHT_VISION, 3600), 0),
                         IDS_POTION_EFFECT_NIGHT_VISION, IDS_POTION_DESC_NIGHTVISION,
                         IDS_SPLASH_POTION_EFFECT_NIGHT_VISION, IDS_LINGERING_POTION_EFFECT_NIGHT_VISION,
                         IDS_TIPPED_ARROW_EFFECT_NIGHT_VISION, IDS_DESC_ITEM_TIPPED_ARROW_NIGHTVISION));
    addPotion(L"long_night_vision",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::NIGHT_VISION, 9600), 1),
                         IDS_POTION_EFFECT_NIGHT_VISION, IDS_POTION_DESC_NIGHTVISION,
                         IDS_SPLASH_POTION_EFFECT_NIGHT_VISION, IDS_LINGERING_POTION_EFFECT_NIGHT_VISION,
                         IDS_TIPPED_ARROW_EFFECT_NIGHT_VISION, IDS_DESC_ITEM_TIPPED_ARROW_NIGHTVISION));
    addPotion(L"invisibility",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::INVISIBILITY, 3600), 2),
                         IDS_POTION_EFFECT_INVISIBILITY, IDS_POTION_DESC_INVISIBILITY, IDS_ITEM_DRIED_POTION_TITLE,
                         IDS_LINGERING_POTION_EFFECT_INVISIBILITY, IDS_TIPPED_ARROW_EFFECT_INVISIBILITY,
                         IDS_DESC_ITEM_TIPPED_ARROW_INVISIBILITY));
    addPotion(L"long_invisibility",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::INVISIBILITY, 9600), 3),
                         IDS_POTION_EFFECT_INVISIBILITY, IDS_POTION_DESC_INVISIBILITY, IDS_ITEM_DRIED_POTION_TITLE,
                         IDS_LINGERING_POTION_EFFECT_INVISIBILITY, IDS_TIPPED_ARROW_EFFECT_INVISIBILITY,
                         IDS_DESC_ITEM_TIPPED_ARROW_INVISIBILITY));
    addPotion(L"leaping", new Potion(effectWithId(new MobEffectInstance(MobEffects::JUMP, 3600), 4),
                                     IDS_POTION_EFFECT_LEAPING, 0x552a6dc4, IDS_SPLASH_POTION_EFFECT_LEAPING,
                                     IDS_LINGERING_POTION_EFFECT_LEAPING, IDS_TIPPED_ARROW_EFFECT_LEAPING, 0x569cc9e7));
    addPotion(L"long_leaping",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::JUMP, 9600), 5), IDS_POTION_EFFECT_LEAPING,
                         0x552a6dc4, IDS_SPLASH_POTION_EFFECT_LEAPING, IDS_LINGERING_POTION_EFFECT_LEAPING,
                         IDS_TIPPED_ARROW_EFFECT_LEAPING, 0x569cc9e7));
    addPotion(L"strong_leaping",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::JUMP, 1800, 1), 6), IDS_POTION_EFFECT_LEAPING,
                         0x552a6dc4, IDS_SPLASH_POTION_EFFECT_LEAPING, IDS_LINGERING_POTION_EFFECT_LEAPING,
                         IDS_TIPPED_ARROW_EFFECT_LEAPING, 0x569cc9e7));
    addPotion(L"fire_resistance",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 3600), 7),
                         IDS_POTION_EFFECT_FIRE_RESISTANCE, IDS_POTION_DESC_FIRERESISTANCE,
                         IDS_SPLASH_POTION_EFFECT_FIRE_RESISTANCE, IDS_LINGERING_POTION_EFFECT_FIRE_RESISTANCE,
                         IDS_TIPPED_ARROW_EFFECT_FIRE_RESISTANCE, IDS_DESC_ITEM_TIPPED_ARROW_FIRERESISTANCE));
    addPotion(L"long_fire_resistance",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 9600), 8),
                         IDS_POTION_EFFECT_FIRE_RESISTANCE, IDS_POTION_DESC_FIRERESISTANCE,
                         IDS_SPLASH_POTION_EFFECT_FIRE_RESISTANCE, IDS_LINGERING_POTION_EFFECT_FIRE_RESISTANCE,
                         IDS_TIPPED_ARROW_EFFECT_FIRE_RESISTANCE, IDS_DESC_ITEM_TIPPED_ARROW_FIRERESISTANCE));
    addPotion(L"swiftness", new Potion(effectWithId(new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 3600), 9),
                                       IDS_POTION_EFFECT_SWIFTNESS, IDS_POTION_DESC_MOVESPEED,
                                       IDS_SPLASH_POTION_EFFECT_SWIFTNESS, IDS_LINGERING_POTION_EFFECT_SWIFTNESS,
                                       IDS_TIPPED_ARROW_EFFECT_SWIFTNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESPEED));
    addPotion(L"long_swiftness", new Potion(effectWithId(new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 9600), 10),
                                            IDS_POTION_EFFECT_SWIFTNESS, IDS_POTION_DESC_MOVESPEED,
                                            IDS_SPLASH_POTION_EFFECT_SWIFTNESS, IDS_LINGERING_POTION_EFFECT_SWIFTNESS,
                                            IDS_TIPPED_ARROW_EFFECT_SWIFTNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESPEED));
    addPotion(L"strong_swiftness",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 1800, 1), 11),
                         IDS_POTION_EFFECT_SWIFTNESS, IDS_POTION_DESC_MOVESPEED, IDS_SPLASH_POTION_EFFECT_SWIFTNESS,
                         IDS_LINGERING_POTION_EFFECT_SWIFTNESS, IDS_TIPPED_ARROW_EFFECT_SWIFTNESS,
                         IDS_DESC_ITEM_TIPPED_ARROW_MOVESPEED));
    addPotion(L"slowness", new Potion(effectWithId(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 1800), 12),
                                      IDS_POTION_EFFECT_SLOWNESS, IDS_POTION_DESC_MOVESLOWDOWN,
                                      IDS_SPLASH_POTION_EFFECT_SLOWNESS, IDS_LINGERING_POTION_EFFECT_SLOWNESS,
                                      IDS_TIPPED_ARROW_EFFECT_SLOWNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESLOWDOWN));
    addPotion(L"long_slowness", new Potion(effectWithId(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 4800), 13),
                                           IDS_POTION_EFFECT_SLOWNESS, IDS_POTION_DESC_MOVESLOWDOWN,
                                           IDS_SPLASH_POTION_EFFECT_SLOWNESS, IDS_LINGERING_POTION_EFFECT_SLOWNESS,
                                           IDS_TIPPED_ARROW_EFFECT_SLOWNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESLOWDOWN));
    addPotion(L"water_breathing",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::WATER_BREATHING, 3600), 14),
                         IDS_POTION_EFFECT_WATER_BREATHING, IDS_POTION_DESC_WATERBREATHING,
                         IDS_SPLASH_POTION_EFFECT_WATER_BREATHING, IDS_LINGERING_POTION_EFFECT_WATER_BREATHING,
                         IDS_TIPPED_ARROW_EFFECT_WATER_BREATHING, IDS_DESC_ITEM_TIPPED_ARROW_WATERBREATHING));
    addPotion(L"long_water_breathing",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::WATER_BREATHING, 9600), 15),
                         IDS_POTION_EFFECT_WATER_BREATHING, IDS_POTION_DESC_WATERBREATHING,
                         IDS_SPLASH_POTION_EFFECT_WATER_BREATHING, IDS_LINGERING_POTION_EFFECT_WATER_BREATHING,
                         IDS_TIPPED_ARROW_EFFECT_WATER_BREATHING, IDS_DESC_ITEM_TIPPED_ARROW_WATERBREATHING));
    addPotion(L"healing",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::HEAL, 1), 16), IDS_POTION_EFFECT_HEALING,
                         IDS_POTION_DESC_HEAL, IDS_SPLASH_POTION_EFFECT_HEALING, IDS_LINGERING_POTION_EFFECT_HEALING,
                         IDS_TIPPED_ARROW_EFFECT_HEALING, IDS_DESC_ITEM_TIPPED_ARROW_HEAL));
    addPotion(L"strong_healing",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::HEAL, 1, 1), 17), IDS_POTION_EFFECT_HEALING,
                         IDS_POTION_DESC_HEAL, IDS_SPLASH_POTION_EFFECT_HEALING, IDS_LINGERING_POTION_EFFECT_HEALING,
                         IDS_TIPPED_ARROW_EFFECT_HEALING, IDS_DESC_ITEM_TIPPED_ARROW_HEAL));
    addPotion(L"harming",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::HARM, 1), 18), IDS_POTION_EFFECT_HARMING,
                         IDS_POTION_DESC_HARM, IDS_SPLASH_POTION_EFFECT_HARMING, IDS_LINGERING_POTION_EFFECT_HARMING,
                         IDS_TIPPED_ARROW_EFFECT_HARMING, IDS_DESC_ITEM_TIPPED_ARROW_HARM));
    addPotion(L"strong_harming",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::HARM, 1, 1), 19), IDS_POTION_EFFECT_HARMING,
                         IDS_POTION_DESC_HARM, IDS_SPLASH_POTION_EFFECT_HARMING, IDS_LINGERING_POTION_EFFECT_HARMING,
                         IDS_TIPPED_ARROW_EFFECT_HARMING, IDS_DESC_ITEM_TIPPED_ARROW_HARM));
    addPotion(L"poison",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::POISON, 900), 20), IDS_POTION_EFFECT_POISON,
                         IDS_POTION_DESC_POISON, IDS_SPLASH_POTION_EFFECT_POISON, IDS_LINGERING_POTION_EFFECT_POISON,
                         IDS_TIPPED_ARROW_EFFECT_POISON, IDS_DESC_ITEM_TIPPED_ARROW_POISON));
    addPotion(L"long_poison",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::POISON, 2400), 21), IDS_POTION_EFFECT_POISON,
                         IDS_POTION_DESC_POISON, IDS_SPLASH_POTION_EFFECT_POISON, IDS_LINGERING_POTION_EFFECT_POISON,
                         IDS_TIPPED_ARROW_EFFECT_POISON, IDS_DESC_ITEM_TIPPED_ARROW_POISON));
    addPotion(L"strong_poison",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::POISON, 450, 1), 22), IDS_POTION_EFFECT_POISON,
                         IDS_POTION_DESC_POISON, IDS_SPLASH_POTION_EFFECT_POISON, IDS_LINGERING_POTION_EFFECT_POISON,
                         IDS_TIPPED_ARROW_EFFECT_POISON, IDS_DESC_ITEM_TIPPED_ARROW_POISON));
    addPotion(L"regeneration",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::REGENERATION, 900), 23),
                         IDS_POTION_EFFECT_REGENERATION, IDS_POTION_DESC_REGENERATION,
                         IDS_SPLASH_POTION_EFFECT_REGENERATION, IDS_LINGERING_POTION_EFFECT_REGENERATION,
                         IDS_TIPPED_ARROW_EFFECT_REGENERATION, IDS_DESC_ITEM_TIPPED_ARROW_REGENERATION));
    addPotion(L"long_regeneration",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::REGENERATION, 2400), 24),
                         IDS_POTION_EFFECT_REGENERATION, IDS_POTION_DESC_REGENERATION,
                         IDS_SPLASH_POTION_EFFECT_REGENERATION, IDS_LINGERING_POTION_EFFECT_REGENERATION,
                         IDS_TIPPED_ARROW_EFFECT_REGENERATION, IDS_DESC_ITEM_TIPPED_ARROW_REGENERATION));
    addPotion(L"strong_regeneration",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::REGENERATION, 450, 1), 25),
                         IDS_POTION_EFFECT_REGENERATION, IDS_POTION_DESC_REGENERATION,
                         IDS_SPLASH_POTION_EFFECT_REGENERATION, IDS_LINGERING_POTION_EFFECT_REGENERATION,
                         IDS_TIPPED_ARROW_EFFECT_REGENERATION, IDS_DESC_ITEM_TIPPED_ARROW_REGENERATION));
    addPotion(L"strength", new Potion(effectWithId(new MobEffectInstance(MobEffects::DAMAGE_BOOST, 3600), 26),
                                      IDS_POTION_EFFECT_STRENGTH, IDS_POTION_DESC_DAMAGEBOOST,
                                      IDS_SPLASH_POTION_EFFECT_STRENGTH, IDS_LINGERING_POTION_EFFECT_STRENGTH,
                                      IDS_TIPPED_ARROW_EFFECT_STRENGTH, IDS_DESC_ITEM_TIPPED_ARROW_DAMAGEBOOST));
    addPotion(L"long_strength", new Potion(effectWithId(new MobEffectInstance(MobEffects::DAMAGE_BOOST, 9600), 27),
                                           IDS_POTION_EFFECT_STRENGTH, IDS_POTION_DESC_DAMAGEBOOST,
                                           IDS_SPLASH_POTION_EFFECT_STRENGTH, IDS_LINGERING_POTION_EFFECT_STRENGTH,
                                           IDS_TIPPED_ARROW_EFFECT_STRENGTH, IDS_DESC_ITEM_TIPPED_ARROW_DAMAGEBOOST));
    addPotion(L"strong_strength", new Potion(effectWithId(new MobEffectInstance(MobEffects::DAMAGE_BOOST, 1800, 1), 28),
                                             IDS_POTION_EFFECT_STRENGTH, IDS_POTION_DESC_DAMAGEBOOST,
                                             IDS_SPLASH_POTION_EFFECT_STRENGTH, IDS_LINGERING_POTION_EFFECT_STRENGTH,
                                             IDS_TIPPED_ARROW_EFFECT_STRENGTH, IDS_DESC_ITEM_TIPPED_ARROW_DAMAGEBOOST));
    addPotion(L"weakness", new Potion(effectWithId(new MobEffectInstance(MobEffects::WEAKNESS, 1800), 29),
                                      IDS_POTION_EFFECT_WEAKNESS, IDS_POTION_DESC_WEAKNESS,
                                      IDS_SPLASH_POTION_EFFECT_WEAKNESS, IDS_LINGERING_POTION_EFFECT_WEAKNESS,
                                      IDS_TIPPED_ARROW_EFFECT_WEAKNESS, IDS_DESC_ITEM_TIPPED_ARROW_WEAKNESS));
    addPotion(L"long_weakness", new Potion(effectWithId(new MobEffectInstance(MobEffects::WEAKNESS, 4800), 30),
                                           IDS_POTION_EFFECT_WEAKNESS, IDS_POTION_DESC_WEAKNESS,
                                           IDS_SPLASH_POTION_EFFECT_WEAKNESS, IDS_LINGERING_POTION_EFFECT_WEAKNESS,
                                           IDS_TIPPED_ARROW_EFFECT_WEAKNESS, IDS_DESC_ITEM_TIPPED_ARROW_WEAKNESS));
    addPotion(L"luck",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::LUCK, 6000), 31), IDS_POTION_EFFECT_LUCK,
                         IDS_POTION_DESC_LUCK, IDS_SPLASH_POTION_EFFECT_LUCK, IDS_LINGERING_POTION_EFFECT_LUCK,
                         IDS_TIPPED_ARROW_EFFECT_LUCK, IDS_DESC_ITEM_TIPPED_ARROW_LUCK));

    std::vector<MobEffectInstance*> turtleEffects;
    MobEffectInstance* turtleEffect;
    turtleEffect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 400, 3);
    turtleEffect->durationEffectId = 33;
    turtleEffects.push_back(turtleEffect);
    turtleEffect = new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 400, 2);
    turtleEffect->durationEffectId = 33;
    turtleEffects.push_back(turtleEffect);
    addPotion(L"turtle_master",
              new Potion(turtleEffects, IDS_POTION_EFFECT_TURTLE_MASTER, IDS_POTION_DESC_TURTLE_MASTER,
                         IDS_SPLASH_POTION_EFFECT_TURTLE_MASTER, IDS_LINGERING_POTION_EFFECT_TURTLE_MASTER,
                         IDS_TIPPED_ARROW_EFFECT_TURTLE_MASTER, IDS_DESC_ITEM_TIPPED_ARROW_TURTLE_MASTER));
    turtleEffects.clear();
    turtleEffect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 800, 3);
    turtleEffect->durationEffectId = 34;
    turtleEffects.push_back(turtleEffect);
    turtleEffect = new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 800, 2);
    turtleEffect->durationEffectId = 34;
    turtleEffects.push_back(turtleEffect);
    addPotion(L"long_turtle_master",
              new Potion(turtleEffects, IDS_POTION_EFFECT_TURTLE_MASTER, IDS_POTION_DESC_TURTLE_MASTER,
                         IDS_SPLASH_POTION_EFFECT_TURTLE_MASTER, IDS_LINGERING_POTION_EFFECT_TURTLE_MASTER,
                         IDS_TIPPED_ARROW_EFFECT_TURTLE_MASTER, IDS_DESC_ITEM_TIPPED_ARROW_TURTLE_MASTER));
    turtleEffects.clear();
    turtleEffect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 400, 5);
    turtleEffect->durationEffectId = 35;
    turtleEffects.push_back(turtleEffect);
    turtleEffect = new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 400, 3);
    turtleEffect->durationEffectId = 35;
    turtleEffects.push_back(turtleEffect);
    addPotion(L"strong_turtle_master",
              new Potion(turtleEffects, IDS_POTION_EFFECT_TURTLE_MASTER, IDS_POTION_DESC_TURTLE_MASTER,
                         IDS_SPLASH_POTION_EFFECT_TURTLE_MASTER, IDS_LINGERING_POTION_EFFECT_TURTLE_MASTER,
                         IDS_TIPPED_ARROW_EFFECT_TURTLE_MASTER, IDS_DESC_ITEM_TIPPED_ARROW_TURTLE_MASTER));
    turtleEffects.clear();

    addPotion(L"wither",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::WITHER, 800, 1), 32), IDS_POTION_EFFECT_WITHER,
                         IDS_POTION_DESC_WITHER, IDS_SPLASH_POTION_EFFECT_WITHER, IDS_LINGERING_POTION_EFFECT_WITHER,
                         IDS_TIPPED_ARROW_EFFECT_WITHER, IDS_DESC_ITEM_TIPPED_ARROW_WITHER));
    addPotion(L"slow_falling",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::SLOW_FALLING, 1800), 36),
                         IDS_POTION_EFFECT_SLOW_FALLING, IDS_POTION_DESC_SLOW_FALLING,
                         IDS_SPLASH_POTION_EFFECT_SLOW_FALLING, IDS_LINGERING_POTION_EFFECT_SLOW_FALLING,
                         IDS_TIPPED_ARROW_EFFECT_SLOW_FALLING, IDS_DESC_ITEM_TIPPED_ARROW_SLOW_FALLING));
    addPotion(L"long_slow_falling",
              new Potion(effectWithId(new MobEffectInstance(MobEffects::SLOW_FALLING, 4800), 37),
                         IDS_POTION_EFFECT_SLOW_FALLING, IDS_POTION_DESC_SLOW_FALLING,
                         IDS_SPLASH_POTION_EFFECT_SLOW_FALLING, IDS_LINGERING_POTION_EFFECT_SLOW_FALLING,
                         IDS_TIPPED_ARROW_EFFECT_SLOW_FALLING, IDS_DESC_ITEM_TIPPED_ARROW_SLOW_FALLING));
}

Item* Potion::getBasePotion(EType type)
{
    switch (type)
    {
    case SPLASH:
        return Items::splashPotion;
    case LINGERING:
        return Items::lingeringPotion;
    default:
        return Items::potion;
    }
}

bool Potion::hasInstantEffects()
{
    if (mEffects.empty())
        return false;
    std::vector<MobEffectInstance*>::iterator it;
    for (it = mEffects.begin(); it != mEffects.end(); ++it)
        if ((*it)->getEffect()->isInstantenous())
            return true;
    return false;
}
