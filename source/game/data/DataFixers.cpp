#include "game/data/DataFixers.hpp"

#include <string>

#include "game/core/ArmorStand.hpp"
#include "game/core/Bat.hpp"
#include "game/core/Blaze.hpp"
#include "game/core/CaveSpider.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/Cow.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/Donkey.hpp"
#include "game/core/ElderGuardian.hpp"
#include "game/core/EnderMan.hpp"
#include "game/core/Endermite.hpp"
#include "game/core/EvocationIllager.hpp"
#include "game/core/Ghast.hpp"
#include "game/core/Giant.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Husk.hpp"
#include "game/core/MagmaCube.hpp"
#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartFurnace.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/MinecartRideable.hpp"
#include "game/core/MinecartSpawner.hpp"
#include "game/core/MinecartTNT.hpp"
#include "game/core/Mule.hpp"
#include "game/core/MushroomCow.hpp"
#include "game/core/Ocelot.hpp"
#include "game/core/OptionsForceVBOFix.hpp"
#include "game/core/OptionsLowerCaseLanguageFix.hpp"
#include "game/core/Pig.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/Shulker.hpp"
#include "game/core/SignTextStrictJsonFix.hpp"
#include "game/core/Silverfish.hpp"
#include "game/core/Slime.hpp"
#include "game/core/SnowMan.hpp"
#include "game/core/Snowball.hpp"
#include "game/core/Spider.hpp"
#include "game/core/Squid.hpp"
#include "game/core/Stray.hpp"
#include "game/core/ThrownEgg.hpp"
#include "game/core/ThrownEnderpearl.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/core/Vex.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/core/Witch.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityArmorStandSilentFix.hpp"
#include "game/entity/mob/EntityElderGuardianSplitFix.hpp"
#include "game/entity/mob/EntityEquipmentToArmorAndHandFix.hpp"
#include "game/entity/mob/EntityHealthFix.hpp"
#include "game/entity/mob/EntityHorseSaddleFix.hpp"
#include "game/entity/mob/EntityHorseSplitFix.hpp"
#include "game/entity/mob/EntityIdFix.hpp"
#include "game/entity/mob/EntityMinecartIdentifiersFix.hpp"
#include "game/entity/mob/EntityPaintingItemFrameDirectionFix.hpp"
#include "game/entity/mob/EntityRedundantChanceTagsFix.hpp"
#include "game/entity/mob/EntityRidingToPassengersFix.hpp"
#include "game/entity/mob/EntityShulkerColorFix.hpp"
#include "game/entity/mob/EntitySkeletonSplitFix.hpp"
#include "game/entity/mob/EntityStringUuidFix.hpp"
#include "game/entity/mob/EntityZombieSplitFix.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/PigZombie.hpp"
#include "game/entity/mob/Skeleton.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/entity/mob/ZombieHorse.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/entity/projectile/DragonFireball.hpp"
#include "game/entity/projectile/LargeFireball.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/BedItemColorFix.hpp"
#include "game/item/ItemBannerColorFix.hpp"
#include "game/item/ItemCookedFishIdFix.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemIdFix.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemPotionFix.hpp"
#include "game/item/ItemShulkerBoxColorFix.hpp"
#include "game/item/ItemSpawnEggFix.hpp"
#include "game/item/ItemTotemRenameFix.hpp"
#include "game/item/ItemWaterPotionFix.hpp"
#include "game/item/ItemWrittenBookPagesStrictJsonFix.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Rabbit.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/BedBlockEntityInjecter.hpp"
#include "game/world/block/BlockEntityIdFix.hpp"
#include "game/world/block/BlockEntityMobSpawnerEntityIdentifiersFix.hpp"
#include "game/world/block/BlockEntityShulkerBoxColorFix.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DropperBlockEntity.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/HopperBlockEntity.hpp"
#include "game/world/block/JukeboxBlock.hpp"
#include "game/world/block/MinecartCommandBlock.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/chunk/McRegionChunkStorage.hpp"
#include "game/world/generation/EntityZombieVillagerTypeFix.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/StructureTemplate.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/ZombieVillager.hpp"

DataFixerUpper* DataFixers::createFixerUpper()
{
    DataFixerUpper* fixerUpper = new DataFixerUpper(922);

    McRegionChunkStorage::addWalkers(fixerUpper);
    LevelData::addWalkers(fixerUpper);
    ServerPlayer::addWalkers(fixerUpper);
    Player::addWalkers(fixerUpper);
    ItemInstance::addDataWalkers(fixerUpper);
    StructureTemplate::addWalkers(fixerUpper);
    Entity::addDataWalkers(fixerUpper);
    ArmorStand::addDataWalkers(fixerUpper);
    Arrow::addDataWalkers(fixerUpper);
    Bat::addDataWalkers(fixerUpper);
    Blaze::addDataWalkers(fixerUpper);
    CaveSpider::addDataWalkers(fixerUpper);
    Chicken::addDataWalkers(fixerUpper);
    Cow::addDataWalkers(fixerUpper);
    Creeper::addDataWalkers(fixerUpper);
    Donkey::addDataWalkers(fixerUpper);
    DragonFireball::addDataWalkers(fixerUpper);
    ElderGuardian::addDataWalkers(fixerUpper);
    EnderDragon::addDataWalkers(fixerUpper);
    EnderMan::addDataWalkers(fixerUpper);
    Endermite::addDataWalkers(fixerUpper);
    EvocationIllager::addDataWalkers(fixerUpper);
    FallingBlockEntity::addDataWalkers(fixerUpper);
    FireworksRocketEntity::addDataWalkers(fixerUpper);
    Ghast::addDataWalkers(fixerUpper);
    Giant::addDataWalkers(fixerUpper);
    Guardian::addDataWalkers(fixerUpper);
    Horse::addDataWalkers(fixerUpper);
    Husk::addDataWalkers(fixerUpper);
    ItemEntity::addDataWalkers(fixerUpper);
    ItemFrame::addDataWalkers(fixerUpper);
    LargeFireball::addDataWalkers(fixerUpper);
    MagmaCube::addDataWalkers(fixerUpper);
    MinecartChest::addDataWalkers(fixerUpper);
    MinecartCommandBlock::addDataWalkers(fixerUpper);
    MinecartFurnace::addDataWalkers(fixerUpper);
    MinecartHopper::addDataWalkers(fixerUpper);
    MinecartRideable::addDataWalkers(fixerUpper);
    MinecartSpawner::addDataWalkers(fixerUpper);
    MinecartTNT::addDataWalkers(fixerUpper);
    Mule::addDataWalkers(fixerUpper);
    MushroomCow::addDataWalkers(fixerUpper);
    Ocelot::addDataWalkers(fixerUpper);
    Pig::addDataWalkers(fixerUpper);
    PigZombie::addDataWalkers(fixerUpper);
    Rabbit::addDataWalkers(fixerUpper);
    Sheep::addDataWalkers(fixerUpper);
    Shulker::addDataWalkers(fixerUpper);
    Silverfish::addDataWalkers(fixerUpper);
    Skeleton::addDataWalkers(fixerUpper);
    SkeletonHorse::addDataWalkers(fixerUpper);
    Slime::addDataWalkers(fixerUpper);
    SmallFireball::addDataWalkers(fixerUpper);
    SnowMan::addDataWalkers(fixerUpper);
    Snowball::addDataWalkers(fixerUpper);
    SpectralArrow::addDataWalkers(fixerUpper);
    Spider::addDataWalkers(fixerUpper);
    Squid::addDataWalkers(fixerUpper);
    Stray::addDataWalkers(fixerUpper);
    ThrownEgg::addDataWalkers(fixerUpper);
    ThrownEnderpearl::addDataWalkers(fixerUpper);
    ThrownExpBottle::addDataWalkers(fixerUpper);
    ThrownPotion::addDataWalkers(fixerUpper);
    TippableArrow::addDataWalkers(fixerUpper);
    Vex::addDataWalkers(fixerUpper);
    Villager::addDataWalkers(fixerUpper);
    VillagerGolem::addDataWalkers(fixerUpper);
    VindicationIllager::addDataWalkers(fixerUpper);
    Witch::addDataWalkers(fixerUpper);
    WitherBoss::addDataWalkers(fixerUpper);
    WitherSkeleton::addDataWalkers(fixerUpper);
    WitherSkull::addDataWalkers(fixerUpper);
    Wolf::addDataWalkers(fixerUpper);
    Zombie::addDataWalkers(fixerUpper);
    ZombieHorse::addDataWalkers(fixerUpper);
    ZombieVillager::addDataWalkers(fixerUpper);
    PistonMovingBlockEntity::addDataWalkers(fixerUpper);
    FlowerPotBlockEntity::addDataWalkers(fixerUpper);
    FurnaceBlockEntity::addDataWalkers(fixerUpper);
    ChestBlockEntity::addDataWalkers(fixerUpper);
    DispenserBlockEntity::addDataWalkers(fixerUpper);
    DropperBlockEntity::addDataWalkers(fixerUpper);
    BrewingStandBlockEntity::addDataWalkers(fixerUpper);
    HopperBlockEntity::addDataWalkers(fixerUpper);
    JukeboxBlock::addDataWalkers(fixerUpper);
    MobSpawnerBlockEntity::addDataWalkers(fixerUpper);
    ShulkerBoxBlockEntity::addDataWalkers(fixerUpper);

    addFixers(fixerUpper);

    return fixerUpper;
}

void DataFixers::addFixers(DataFixerUpper* fixerUpper)
{
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityEquipmentToArmorAndHandFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_BlockEntity, new SignTextStrictJsonFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemIdFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemPotionFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemSpawnEggFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityMinecartIdentifiersFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_BlockEntity, new BlockEntityMobSpawnerEntityIdentifiersFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityStringUuidFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityHealthFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityHorseSaddleFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityPaintingItemFrameDirectionFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityRedundantChanceTagsFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityRidingToPassengersFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityArmorStandSilentFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemWrittenBookPagesStrictJsonFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemCookedFishIdFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityZombieVillagerTypeFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Options, new OptionsForceVBOFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityElderGuardianSplitFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntitySkeletonSplitFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityZombieSplitFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityHorseSplitFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_BlockEntity, new BlockEntityIdFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityIdFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemBannerColorFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemWaterPotionFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Entity, new EntityShulkerColorFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemShulkerBoxColorFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_BlockEntity, new BlockEntityShulkerBoxColorFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Options, new OptionsLowerCaseLanguageFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new ItemTotemRenameFix());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_Chunk, new BedBlockEntityInjecter());
    fixerUpper->addFixer(DataFixTypes::eDataFixTypes_ItemInstance, new BedItemColorFix());
}

CompoundTag* DataFixers::updateItem(DataFixer* fixer, CompoundTag* tag, int version, const std::wstring& name)
{
    if (tag->contains(name, Tag::TAG_COMPOUND))
    {
        tag->put(name, fixer->update(DataFixTypes::eDataFixTypes_ItemInstance, tag->getCompound(name), version));
    }

    return tag;
}

CompoundTag* DataFixers::updateItems(DataFixer* fixer, CompoundTag* tag, int version, const std::wstring& name)
{
    if (tag->contains(name, Tag::TAG_LIST))
    {
        ListTag* items = tag->getList(name, Tag::TAG_COMPOUND);
        for (int i = 0; i < items->size(); ++i)
        {
            items->set(i, fixer->update(DataFixTypes::eDataFixTypes_ItemInstance, items->getCompound(i), version));
        }
    }

    return tag;
}
