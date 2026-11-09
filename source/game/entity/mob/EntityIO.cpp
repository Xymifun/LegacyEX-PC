#include "game/entity/mob/EntityIO.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/core/ElderGuardian.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/core/Stray.hpp"
#include "game/core/ThrownEgg.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/core/Snowball.hpp"
#include "game/entity/projectile/LargeFireball.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/core/ThrownEnderpearl.hpp"
#include "game/core/EyeOfEnderSignal.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/core/Husk.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/core/ShulkerBullet.hpp"
#include "game/entity/projectile/DragonFireball.hpp"
#include "game/world/generation/ZombieVillager.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/entity/mob/ZombieHorse.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/Donkey.hpp"
#include "game/core/Mule.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/core/EvocationIllager.hpp"
#include "game/core/Vex.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/core/IllusionIllager.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/core/Dolphin.hpp"
#include "game/core/Boat.hpp"
#include "game/core/MinecartRideable.hpp"
#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartFurnace.hpp"
#include "game/core/MinecartTNT.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/MinecartSpawner.hpp"
#include "game/core/Creeper.hpp"
#include "game/entity/mob/Skeleton.hpp"
#include "game/core/Spider.hpp"
#include "game/core/Giant.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/core/Slime.hpp"
#include "game/core/Ghast.hpp"
#include "game/entity/mob/PigZombie.hpp"
#include "game/core/EnderMan.hpp"
#include "game/core/CaveSpider.hpp"
#include "game/core/Silverfish.hpp"
#include "game/core/Blaze.hpp"
#include "game/core/MagmaCube.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/core/Bat.hpp"
#include "game/core/Witch.hpp"
#include "game/core/Endermite.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Shulker.hpp"
#include "game/core/Pig.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/Cow.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/Squid.hpp"
#include "game/core/Wolf.hpp"
#include "game/core/MushroomCow.hpp"
#include "game/core/SnowMan.hpp"
#include "game/core/Ocelot.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/util/Rabbit.hpp"
#include "game/core/PolarBear.hpp"
#include "game/core/Llama.hpp"
#include "game/core/LlamaSpit.hpp"
#include "game/core/Parrot.hpp"
#include "game/core/TropicalFish.hpp"
#include "game/core/Fish.hpp"
#include "game/core/Pufferfish.hpp"
#include "game/core/Salmon.hpp"
#include "game/core/Turtle.hpp"
#include "game/core/Drowned.hpp"
#include "game/core/Phantom.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/core/EnderCrystal.hpp"

namespace
{
typedef boost::unordered_map<unsigned long long, EntityIO::Creator> Creators;

ResourceLocation sLightningBolt(std::wstring(L"LightningBolt"));
ResourceLocation sPlayer(std::wstring(L"Player"));
}

EntityIO::EntityRegistry EntityIO::sRegistry;
EntityIO::SpawnInfo EntityIO::sSpawnInfo;

namespace
{
boost::unordered_set<ResourceLocation> sEntityTypes;
std::vector<std::wstring> sTranslationNames;
Creators sCreators;

}


EntityIO::SpawnableMobInfo::SpawnableMobInfo(int infoId, const ResourceLocation& infoLocation,
                                             eMinecraftColour infoColour, eMinecraftColour infoSecondaryColour,
                                             eMinecraftIconOverride infoIcon)
{
    id = infoId;
    location = ResourceLocation(infoLocation);
    colour = infoColour;
    secondaryColour = infoSecondaryColour;
    type = 0;
    icon = infoIcon;
    flags = 0;
}

EntityIO::SpawnableMobInfo::SpawnableMobInfo(const SpawnableMobInfo& other)
    : id(other.id),
      location(other.location),
      colour(other.colour),
      secondaryColour(other.secondaryColour),
      icon(other.icon),
      type(other.type),
      flags(other.flags)
{
}

ResourceLocation EntityIO::getKey(unsigned long long entityType)
{
    return sRegistry.getKey(entityType);
}

boost::shared_ptr<Entity> EntityIO::newEntity(unsigned long long entityType, Level* level)
{
    if (!eTYPE_DERIVED_FROM(EntityType::ENTITY, entityType))
    {
        return boost::shared_ptr<Entity>();
    }
    boost::shared_ptr<Entity> entity;
    Creators::const_iterator it = sCreators.find(entityType);
    if (it != sCreators.end())
    {
        if (it->second != NULL)
        {
            entity = boost::shared_ptr<Entity>((it->second)(level));
        }
        if (entity != NULL && eTYPE_DERIVED_FROM(EntityType::ENDER_DRAGON, entity->GetType()))
        {
            boost::static_pointer_cast<EnderDragon>(entity)->AddParts();
        }
    }
    return entity;
}

ResourceLocation EntityIO::getKey(const boost::shared_ptr<Entity>& entity)
{
    return getKey(entity->GetType());
}

std::wstring EntityIO::getTranslationName(boost::shared_ptr<Entity> entity)
{
    int id = sRegistry.getId(entity->GetType());
    if (id != -1)
    {
        return sTranslationNames[id];
    }
    return std::wstring(L"");
}

unsigned long long EntityIO::byId(int id)
{
    return sRegistry.byId(id);
}

unsigned long long EntityIO::byString(const std::wstring& name)
{
    return sRegistry.get(ResourceLocation(name));
}

boost::shared_ptr<Entity> EntityIO::newEntity(int id, Level* level)
{
    return newEntity(byId(id), level);
}

boost::shared_ptr<Entity> EntityIO::newEntity(const ResourceLocation& location, Level* level)
{
    return newEntity(sRegistry.get(location), level);
}

boost::unordered_set<ResourceLocation>* EntityIO::getAllEntityTypes()
{
    return &sEntityTypes;
}

bool EntityIO::isValidEntityType(const ResourceLocation& location)
{
    boost::unordered_set<ResourceLocation>* types = getAllEntityTypes();
    bool found = types->find(location) != types->end();
    return sPlayer.equals(&location) || found;
}

boost::shared_ptr<Entity> EntityIO::newEntity(CompoundTag* tag, Level* level)
{
    if (!isValidEntityType(ResourceLocation(tag->getString(L"id"))))
    {
        return boost::shared_ptr<Entity>();
    }
    if (tag->getString(L"id").compare(L"Minecart") == 0)
    {
        tag->putString(L"id", std::wstring(Minecart::Type::NAMES[Minecart::Type::VALUES[tag->getInt(L"Type")]->mId]));
        tag->remove(L"Type");
    }
    ResourceLocation location(tag->getString(L"id"));
    boost::shared_ptr<Entity> entity = newEntity(sRegistry.get(location), level);
    if (entity)
    {
        entity->load(tag);
    }
    return entity;
}

bool EntityIO::isEntityOfType(boost::shared_ptr<Entity> entity, const ResourceLocation& location)
{
    ResourceLocation key = getKey(entity->GetType());
    if (!key.isDefault())
    {
        return key.equals(&location);
    }
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        return sPlayer.equals(&location);
    }
    if (eTYPE_DERIVED_FROM(EntityType::LIGHTNING_BOLT, entity->GetType()))
    {
        return sLightningBolt.equals(&location);
    }
    return false;
}

int EntityIO::getNameDisplayId(const std::wstring& name)
{
    int id = -1;
    ResourceLocation location(name);
    SpawnInfo::const_iterator it = sSpawnInfo.find(location);
    if (it != sSpawnInfo.end())
    {
        id = it->second->id;
    }
    return id;
}

int EntityIO::eTypeToIoid(unsigned long long entityType)
{
    return sRegistry.getId(entityType);
}

EntityIO::SpawnableMobInfo EntityIO::addSpawnInfo(int id, std::wstring name, eMinecraftColour colour,
                                                  eMinecraftColour secondaryColour, eMinecraftIconOverride icon)
{
    ResourceLocation location(name);
    SpawnableMobInfo* info = new SpawnableMobInfo(id, location, colour, secondaryColour, icon);
    sSpawnInfo[location] = info;
    return *sSpawnInfo[location];
}

void EntityIO::Register(Creator creator, int id, const std::wstring& name, unsigned long long entityType,
                        const std::wstring& translationName)
{
    sCreators.find(entityType);
    sRegistry.getId(entityType);
    ResourceLocation location(name);
    sRegistry.registerMapping(id, location, entityType);
    sEntityTypes.insert(location);
    while (sTranslationNames.size() <= static_cast<unsigned int>(id))
    {
        sTranslationNames.push_back(std::wstring(L""));
    }
    sTranslationNames.at(id) = translationName;
    sCreators.insert(std::make_pair(entityType, creator));
}

void EntityIO::bootStrap()
{
    Register(ItemEntity::create, 1, L"item", EntityType::ITEM_ENTITY, L"Item");
    Register(ExperienceOrb::create, 2, L"xp_orb", EntityType::EXPERIENCE_ORB, L"XPOrb");
    Register(AreaEffectCloud::create, 3, L"area_effect_cloud", EntityType::AREA_EFFECT_CLOUD, L"AreaEffectCloud");
    Register(ElderGuardian::create, 4, L"elder_guardian", EntityType::ELDER_GUARDIAN, L"ElderGuardian");
    Register(WitherSkeleton::create, 5, L"wither_skeleton", EntityType::WITHER_SKELETON, L"WitherSkeleton");
    Register(Stray::create, 6, L"stray", EntityType::STRAY, L"Stray");
    Register(ThrownEgg::create, 7, L"egg", EntityType::THROWN_EGG, L"ThrownEgg");
    Register(LeashFenceKnotEntity::create, 8, L"leash_knot", EntityType::LEASH_FENCE_KNOT_ENTITY, L"LeashKnot");
    Register(Painting::create, 9, L"painting", EntityType::PAINTING, L"Painting");
    Register(TippableArrow::create, 10, L"arrow", EntityType::TIPPABLE_ARROW, L"Arrow");
    Register(Snowball::create, 11, L"snowball", EntityType::SNOWBALL, L"Snowball");
    Register(LargeFireball::create, 12, L"fireball", EntityType::LARGE_FIREBALL, L"Fireball");
    Register(SmallFireball::create, 13, L"small_fireball", EntityType::SMALL_FIREBALL, L"SmallFireball");
    Register(ThrownEnderpearl::create, 14, L"ender_pearl", EntityType::THROWN_ENDERPEARL, L"ThrownEnderpearl");
    Register(EyeOfEnderSignal::create, 15, L"eye_of_ender_signal", EntityType::EYE_OF_ENDER_SIGNAL,
             L"EyeOfEnderSignal");
    Register(ThrownPotion::create, 16, L"potion", EntityType::THROWN_POTION, L"ThrownPotion");
    Register(ThrownExpBottle::create, 17, L"xp_bottle", EntityType::THROWN_EXP_BOTTLE, L"ThrownExpBottle");
    Register(ItemFrame::create, 18, L"item_frame", EntityType::ITEM_FRAME, L"ItemFrame");
    Register(WitherSkull::create, 19, L"wither_skull", EntityType::WITHER_SKULL, L"WitherSkull");
    Register(PrimedTnt::create, 20, L"tnt", EntityType::PRIMED_TNT, L"PrimedTnt");
    Register(FallingBlockEntity::create, 21, L"falling_block", EntityType::FALLING_BLOCK_ENTITY, L"FallingSand");
    Register(FireworksRocketEntity::create, 22, L"fireworks_rocket", EntityType::FIREWORKS_ROCKET_ENTITY,
             L"FireworksRocketEntity");
    Register(Husk::create, 23, L"husk", EntityType::HUSK, L"Husk");
    Register(SpectralArrow::create, 24, L"spectral_arrow", EntityType::SPECTRAL_ARROW, L"SpectralArrow");
    Register(ShulkerBullet::create, 25, L"shulker_bullet", EntityType::SHULKER_BULLET, L"ShulkerBullet");
    Register(DragonFireball::create, 26, L"dragon_fireball", EntityType::DRAGON_FIREBALL, L"DragonFireball");
    Register(ZombieVillager::create, 27, L"zombie_villager", EntityType::ZOMBIE_VILLAGER, L"ZombieVillager");
    Register(SkeletonHorse::create, 28, L"skeleton_horse", EntityType::SKELETON_HORSE, L"SkeletonHorse");
    Register(ZombieHorse::create, 29, L"zombie_horse", EntityType::ZOMBIE_HORSE, L"ZombieHorse");
    Register(ArmorStand::create, 30, L"armor_stand", EntityType::ARMOR_STAND, L"ArmorStand");
    Register(Donkey::create, 31, L"donkey", EntityType::DONKEY, L"Donkey");
    Register(Mule::create, 32, L"mule", EntityType::MULE, L"Mule");
    Register(EvocationFangs::create, 33, L"evocation_fangs", EntityType::EVOCATION_FANGS, L"EvocationFangs");
    Register(EvocationIllager::create, 34, L"evocation_illager", EntityType::EVOCATION_ILLAGER, L"EvocationIllager");
    Register(Vex::create, 35, L"vex", EntityType::VEX, L"Vex");
    Register(VindicationIllager::create, 36, L"vindication_illager", EntityType::VINDICATION_ILLAGER,
             L"VindicationIllager");
    Register(IllusionIllager::create, 37, L"illusion_illager", EntityType::ILLUSION_ILLAGER, L"IllusionIllager");
    Register(ThrownTrident::create, 38, L"trident", EntityType::THROWN_TRIDENT, L"Trident");
    Register(Dolphin::create, 39, L"dolphin", EntityType::DOLPHIN, L"Dolphin");
    Register(Boat::create, 41, L"boat", EntityType::BOAT, L"Boat");
    Register(MinecartRideable::create, 42, L"minecart", EntityType::MINECART_RIDEABLE,
             Minecart::Type::RIDEABLE->getName());
    Register(MinecartChest::create, 43, L"chest_minecart", EntityType::MINECART_CHEST,
             Minecart::Type::CHEST->getName());
    Register(MinecartFurnace::create, 44, L"furnace_minecart", EntityType::MINECART_FURNACE,
             Minecart::Type::FURNACE->getName());
    Register(MinecartTNT::create, 45, L"tnt_minecart", EntityType::MINECART_TNT, Minecart::Type::TNT->getName());
    Register(MinecartHopper::create, 46, L"hopper_minecart", EntityType::MINECART_HOPPER,
             Minecart::Type::HOPPER->getName());
    Register(MinecartSpawner::create, 47, L"spawner_minecart", EntityType::MINECART_SPAWNER,
             Minecart::Type::SPAWNER->getName());
    Register(Creeper::create, 50, L"creeper", EntityType::CREEPER, L"Creeper");
    Register(Skeleton::create, 51, L"skeleton", EntityType::SKELETON, L"Skeleton");
    Register(Spider::create, 52, L"spider", EntityType::SPIDER, L"Spider");
    Register(Giant::create, 53, L"giant", EntityType::GIANT, L"Giant");
    Register(Zombie::create, 54, L"zombie", EntityType::ZOMBIE, L"Zombie");
    Register(Slime::create, 55, L"slime", EntityType::SLIME, L"Slime");
    Register(Ghast::create, 56, L"ghast", EntityType::GHAST, L"Ghast");
    Register(PigZombie::create, 57, L"zombie_pigman", EntityType::PIG_ZOMBIE, L"PigZombie");
    Register(EnderMan::create, 58, L"enderman", EntityType::ENDER_MAN, L"Enderman");
    Register(CaveSpider::create, 59, L"cave_spider", EntityType::CAVE_SPIDER, L"CaveSpider");
    Register(Silverfish::create, 60, L"silverfish", EntityType::SILVERFISH, L"Silverfish");
    Register(Blaze::create, 61, L"blaze", EntityType::BLAZE, L"Blaze");
    Register(MagmaCube::create, 62, L"magma_cube", EntityType::MAGMA_CUBE, L"LavaSlime");
    Register(EnderDragon::create, 63, L"ender_dragon", EntityType::ENDER_DRAGON, L"EnderDragon");
    Register(WitherBoss::create, 64, L"wither", EntityType::WITHER_BOSS, L"WitherBoss");
    Register(Bat::create, 65, L"bat", EntityType::BAT, L"Bat");
    Register(Witch::create, 66, L"witch", EntityType::WITCH, L"Witch");
    Register(Endermite::create, 67, L"endermite", EntityType::ENDERMITE, L"Endermite");
    Register(Guardian::create, 68, L"guardian", EntityType::GUARDIAN, L"Guardian");
    Register(Shulker::create, 69, L"shulker", EntityType::SHULKER, L"Shulker");
    Register(Pig::create, 90, L"pig", EntityType::PIG, L"Pig");
    Register(Sheep::create, 91, L"sheep", EntityType::SHEEP, L"Sheep");
    Register(Cow::create, 92, L"cow", EntityType::COW, L"Cow");
    Register(Chicken::create, 93, L"chicken", EntityType::CHICKEN, L"Chicken");
    Register(Squid::create, 94, L"squid", EntityType::SQUID, L"Squid");
    Register(Wolf::create, 95, L"wolf", EntityType::WOLF, L"Wolf");
    Register(MushroomCow::create, 96, L"mooshroom", EntityType::MUSHROOM_COW, L"MushroomCow");
    Register(SnowMan::create, 97, L"snowman", EntityType::SNOW_MAN, L"SnowMan");
    Register(Ocelot::create, 98, L"ocelot", EntityType::OCELOT, L"Ozelot");
    Register(VillagerGolem::create, 99, L"villager_golem", EntityType::VILLAGER_GOLEM, L"VillagerGolem");
    Register(Horse::create, 100, L"horse", EntityType::HORSE, L"Horse");
    Register(Rabbit::create, 101, L"rabbit", EntityType::RABBIT, L"Rabbit");
    Register(PolarBear::create, 102, L"polar_bear", EntityType::POLAR_BEAR, L"PolarBear");
    Register(Llama::create, 103, L"llama", EntityType::LLAMA, L"Llama");
    Register(LlamaSpit::create, 104, L"llama_spit", EntityType::LLAMA_SPIT, L"LlamaSpit");
    Register(Parrot::create, 105, L"parrot", EntityType::PARROT, L"Parrot");
    Register(TropicalFish::create, 106, L"tropicalfish", EntityType::TROPICAL_FISH, L"TropicalFish");
    Register(Fish::create, 107, L"fish", EntityType::FISH, L"Fish");
    Register(Pufferfish::create, 108, L"pufferfish", EntityType::PUFFERFISH, L"Pufferfish");
    Register(Salmon::create, 109, L"salmon", EntityType::SALMON, L"Salmon");
    Register(Turtle::create, 110, L"turtle", EntityType::TURTLE, L"Turtle");
    Register(Drowned::create, 111, L"drowned", EntityType::DROWNED, L"Drowned");
    Register(Phantom::create, 112, L"phantom", EntityType::PHANTOM, L"Phantom");
    Register(Villager::create, 120, L"villager", EntityType::VILLAGER, L"Villager");
    Register(EnderCrystal::create, 200, L"ender_crystal", EntityType::ENDER_CRYSTAL, L"EnderCrystal");

    addSpawnInfo(static_cast<int>(IDS_BAT), L"bat", eMinecraftColour_Mob_Bat_Colour1, eMinecraftColour_Mob_Bat_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_BLAZE), L"blaze", eMinecraftColour_Mob_Blaze_Colour1,
                 eMinecraftColour_Mob_Blaze_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_CAVE_SPIDER), L"cave_spider", eMinecraftColour_Mob_CaveSpider_Colour1,
                 eMinecraftColour_Mob_CaveSpider_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_CHICKEN), L"chicken", eMinecraftColour_Mob_Chicken_Colour1,
                 eMinecraftColour_Mob_Chicken_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_COW), L"cow", eMinecraftColour_Mob_Cow_Colour1, eMinecraftColour_Mob_Cow_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_CREEPER), L"creeper", eMinecraftColour_Mob_Creeper_Colour1,
                 eMinecraftColour_Mob_Creeper_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_DONKEY), L"donkey", eMinecraftColour_Mob_Donkey_Colour1,
                 eMinecraftColour_Mob_Donkey_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_ELDER_GUARDIAN), L"elder_guardian", eMinecraftColour_Mob_Elder_Guardian_Colour1,
                 eMinecraftColour_Mob_Elder_Guardian_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_ENDERMAN), L"enderman", eMinecraftColour_Mob_Enderman_Colour1,
                 eMinecraftColour_Mob_Enderman_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_ENDERMITE), L"endermite", eMinecraftColour_Mob_Endermite_Color1,
                 eMinecraftColour_Mob_Endermite_Color2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(0xD9AA9869U), L"evocation_illager", eMinecraftColour_Mob_Evocation_Illager_Colour1,
                 eMinecraftColour_Mob_Evocation_Illager_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_GHAST), L"ghast", eMinecraftColour_Mob_Ghast_Colour1,
                 eMinecraftColour_Mob_Ghast_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_GUARDIAN), L"guardian", eMinecraftColour_Mob_Guardian_Color1,
                 eMinecraftColour_Mob_Guardian_Color2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_HORSE), L"horse", eMinecraftColour_Mob_Horse_Colour1,
                 eMinecraftColour_Mob_Horse_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_HUSK), L"husk", eMinecraftColour_Mob_Husk_Colour1,
                 eMinecraftColour_Mob_Husk_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_LLAMA), L"llama", eMinecraftColour_Mob_Llama_Colour1,
                 eMinecraftColour_Mob_Llama_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(0x9E8D900FU), L"magma_cube", eMinecraftColour_Mob_LavaSlime_Colour1,
                 eMinecraftColour_Mob_LavaSlime_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(0x2775807CU), L"mooshroom", eMinecraftColour_Mob_MushroomCow_Colour1,
                 eMinecraftColour_Mob_MushroomCow_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_MULE), L"mule", eMinecraftColour_Mob_Mule_Colour1,
                 eMinecraftColour_Mob_Mule_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(0xAA44AD8BU), L"ocelot", eMinecraftColour_Mob_Ocelot_Colour1,
                 eMinecraftColour_Mob_Ocelot_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_PARROT), L"parrot", eMinecraftColour_Mob_Parrot_Colour1,
                 eMinecraftColour_Mob_Parrot_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_PIG), L"pig", eMinecraftColour_Mob_Pig_Colour1, eMinecraftColour_Mob_Pig_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_POLARBEAR), L"polar_bear", eMinecraftColour_Mob_PolarBear_Colour1,
                 eMinecraftColour_Mob_PolarBear_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_RABBIT), L"rabbit", eMinecraftColour_Mob_Rabbit_Colour1,
                 eMinecraftColour_Mob_Rabbit_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SHEEP), L"sheep", eMinecraftColour_Mob_Sheep_Colour1,
                 eMinecraftColour_Mob_Sheep_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SHULKER), L"shulker", eMinecraftColour_Mob_Shulker_Colour1,
                 eMinecraftColour_Mob_Shulker_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SILVERFISH), L"silverfish", eMinecraftColour_Mob_Silverfish_Colour1,
                 eMinecraftColour_Mob_Silverfish_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SKELETON), L"skeleton", eMinecraftColour_Mob_Skeleton_Colour1,
                 eMinecraftColour_Mob_Skeleton_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SKELETON_HORSE), L"skeleton_horse", eMinecraftColour_Mob_Skeleton_Horse_Colour1,
                 eMinecraftColour_Mob_Skeleton_Horse_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SLIME), L"slime", eMinecraftColour_Mob_Slime_Colour1,
                 eMinecraftColour_Mob_Slime_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SPIDER), L"spider", eMinecraftColour_Mob_Spider_Colour1,
                 eMinecraftColour_Mob_Spider_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SQUID), L"squid", eMinecraftColour_Mob_Squid_Colour1,
                 eMinecraftColour_Mob_Squid_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(0xC9DE5707U), L"stray", eMinecraftColour_Mob_Stray_Colour1,
                 eMinecraftColour_Mob_Stray_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_TURTLE), L"turtle", eMinecraftColour_Mob_Turtle_Colour1,
                 eMinecraftColour_Mob_Turtle_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_DOLPHIN), L"dolphin", eMinecraftColour_Mob_Dolphin_Colour1,
                 eMinecraftColour_Mob_Dolphin_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_VEX), L"vex", eMinecraftColour_Mob_Vex_Colour1, eMinecraftColour_Mob_Vex_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_VILLAGER), L"villager", eMinecraftColour_Mob_Villager_Colour1,
                 eMinecraftColour_Mob_Villager_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(0xF3D4F291U), L"vindication_illager",
                 eMinecraftColour_Mob_Vindication_Illager_Colour1, eMinecraftColour_Mob_Vindication_Illager_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_WITCH), L"witch", eMinecraftColour_Mob_Witch_Colour1,
                 eMinecraftColour_Mob_Witch_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_WITHER_SKELETON), L"wither_skeleton",
                 eMinecraftColour_Mob_Wither_Skeleton_Colour1, eMinecraftColour_Mob_Wither_Skeleton_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_WOLF), L"wolf", eMinecraftColour_Mob_Wolf_Colour1,
                 eMinecraftColour_Mob_Wolf_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_ZOMBIE), L"zombie", eMinecraftColour_Mob_Zombie_Colour1,
                 eMinecraftColour_Mob_Zombie_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_ZOMBIE_HORSE), L"zombie_horse", eMinecraftColour_Mob_Zombie_Horse_Colour1,
                 eMinecraftColour_Mob_Zombie_Horse_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(0xF566D3CCU), L"zombie_pigman", eMinecraftColour_Mob_PigZombie_Colour1,
                 eMinecraftColour_Mob_PigZombie_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_ZOMBIE_VILLAGER), L"zombie_villager",
                 eMinecraftColour_Mob_Zombie_Villager_Colour1, eMinecraftColour_Mob_Zombie_Villager_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_TROPICALFISH), L"tropicalfish", eMinecraftColour_Mob_Tropical_Colour1,
                 eMinecraftColour_Mob_Tropical_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_COD), L"fish", eMinecraftColour_Mob_Cod_Colour1, eMinecraftColour_Mob_Cod_Colour2,
                 eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_PUFFERFISH), L"pufferfish", eMinecraftColour_Mob_Pufferfish_Colour1,
                 eMinecraftColour_Mob_Pufferfish_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_SALMON), L"salmon", eMinecraftColour_Mob_Salmon_Colour1,
                 eMinecraftColour_Mob_Salmon_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_DROWNED), L"drowned", eMinecraftColour_Mob_Drowned_Colour1,
                 eMinecraftColour_Mob_Drowned_Colour2, eMinecraftIconOverride_NONE);
    addSpawnInfo(static_cast<int>(IDS_PHANTOM), L"phantom", eMinecraftColour_Mob_Phantom_Colour1,
                 eMinecraftColour_Mob_Phantom_Colour2, eMinecraftIconOverride_NONE);
    sEntityTypes.insert(sLightningBolt);
}
