#include "game/entity/mob/ZombieHorse.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

ZombieHorse::ZombieHorse(Level* level) : AbstractHorse(level)
{
    fjDerivedCtorCalls();
}

Entity* ZombieHorse::create(Level* level)
{
    return new ZombieHorse(level);
}

void ZombieHorse::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractHorse::addDataWalkers(fixer, EntityType::ZOMBIE_HORSE);
}

void ZombieHorse::registerAttributes()
{
    AbstractHorse::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(15.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
    getAttribute(JUMP_STRENGTH)->setBaseValue(generateRandomJumpStrength());
}

void ZombieHorse::registerGoals()
{
    AbstractHorse::registerGoals();
    mGoalSelector.removeGoal(mFloatGoal);
    mGoalSelector.removeGoal(mWaterAvoidingRandomStrollGoal);
    mGoalSelector.addGoal(6, new SinkInWaterRandomStrollGoal(this, 0.7), true);
}

EMobType ZombieHorse::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

boost::shared_ptr<AgableMob> ZombieHorse::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<ZombieHorse>(new ZombieHorse(mLevel));
}

const SoundEvent* ZombieHorse::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return SoundEvents::ZOMBIE_HORSE_AMBIENT;
}

const SoundEvent* ZombieHorse::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::ZOMBIE_HORSE_DEATH;
}

const SoundEvent* ZombieHorse::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::ZOMBIE_HORSE_HURT;
}

ResourceLocation* ZombieHorse::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE_HORSE;
}

bool ZombieHorse::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    bool notEmpty = !item->isEmpty();
    if (notEmpty && item->getItem() == Items::spawnEgg)
        return AbstractHorse::mobInteract(player, hand);
    if (!isTamed())
        return false;
    if (isBaby())
        return AbstractHorse::mobInteract(player, hand);
    if (player->isSneaking())
    {
        openInventory(player);
        return true;
    }
    if (isVehicle())
        return AbstractHorse::mobInteract(player, hand);
    if (notEmpty)
    {
        if (!isSaddled() && item->getItem() == Items::saddle)
        {
            openInventory(player);
            return true;
        }
        if (item->interactEnemy(player, boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand))
            return true;
    }
    doPlayerRide(player);
    return true;
}

unsigned int ZombieHorse::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bAllowedToInteract)
    {
        unsigned int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
        bool horseArmor = itemId == ItemId::IRON_HORSE_ARMOR || itemId == ItemId::GOLDEN_HORSE_ARMOR
                          || itemId == ItemId::DIAMOND_HORSE_ARMOR || itemId == ItemId::LEATHER_HORSE_ARMOR;

        if (itemId == ItemId::SPAWN_EGG)
        {
            if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == EntityType::ZOMBIE_HORSE)
                return IDS_TOOLTIPS_PLACE;
            if (isLeashedToPlayer(tooltipData.m_pPlayer))
                return IDS_TOOLTIPS_UNLEASH;
        }
        else
        {
            if (isLeashedToPlayer(tooltipData.m_pPlayer))
                return IDS_TOOLTIPS_UNLEASH;
            if (itemId == ItemId::LEAD)
            {
                if (!isLeashed())
                    return IDS_TOOLTIPS_LEASH;
            }
            else
            {
                if (itemId == ItemId::NAME_TAG && !(tooltipData.m_pPlayer->isSneaking() && isTamed()))
                {
                    if (!tooltipData.m_pItemInstance->hasCustomHoverName())
                        return IDS_NONE;
                    if (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) != 0)
                        return IDS_TOOLTIPS_NAME;
                    return IDS_NONE;
                }

                if (isTamed())
                {
                    if (tooltipData.m_pPlayer->isSneaking() || itemId == ItemId::SADDLE && !isSaddled()
                        || wearsArmor() && horseArmor)
                        return IDS_TOOLTIPS_OPEN;
                    if (getPassengers()->empty())
                        return IDS_TOOLTIPS_MOUNT;
                }
            }
        }
    }
    return IDS_NONE;
}

bool ZombieHorse::breathesWater()
{
    return true;
}

unsigned long long ZombieHorse::GetType()
{
    return EntityType::ZOMBIE_HORSE;
}
