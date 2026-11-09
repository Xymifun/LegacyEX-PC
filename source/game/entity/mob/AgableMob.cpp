#include "game/entity/mob/AgableMob.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"


#include <string>

#include <boost/shared_ptr.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/Items.hpp"
#include "game/core/Abilities.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

const EntityDataAccessor<bool>* AgableMob::DATA_BABY_ID = NULL;
const EntityDataAccessor<bool>* AgableMob::DATA_BABY_ID_2 = NULL;

AgableMob::AgableMob(Level* level) : PathfinderMob(level)
{
    mBaseWidth = -1.0f;
    mBaseHeight = 0.0f;
    mForcedAgeTimer = 0;
    mForcedAge = 0;
    mAge = 0;
}

void AgableMob::staticCtor()
{
    DATA_BABY_ID = SynchedEntityData::defineId(EntityType::AGABLE_MOB, EntityType::PATHFINDER_MOB,
                                               EntityDataSerializers::BOOLEAN);
    DATA_BABY_ID_2 = SynchedEntityData::defineId(EntityType::AGABLE_MOB, EntityType::PATHFINDER_MOB,
                                                 EntityDataSerializers::BOOLEAN);
}

bool AgableMob::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->getItem() == Items::spawnEgg && !mLevel->mIsClientSide)
    {
        const unsigned long long type = EntityIO::sRegistry.get(SpawnEggItem::getNamedIdFrom(itemInstance));
        if ((type != 0 ? eTYPE_DERIVED_FROM(EntityType::AGABLE_MOB, type) : false) && type == GetType())
        {
            int error = -1;
            boost::shared_ptr<Entity> spawnCheck = SpawnEggItem::canSpawn(SpawnEggItem::getNamedIdFrom(itemInstance),
                                                                          mLevel, &error);
            if (spawnCheck != NULL && mLevel->canCreateMore(GetType(), Level::eSpawnType_Egg))
            {
                boost::shared_ptr<AgableMob> child = getBreedOffspring(
                    boost::static_pointer_cast<AgableMob>(shared_from_this()));
                if (child != NULL)
                {
                    child->setAge(-24000);
                    child->moveTo(mX, mY, mZ, 0.0f, 0.0f);
                    child->setDespawnProtected();
                    mLevel->addEntity(child);
                    if (itemInstance->hasCustomHoverName())
                        child->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                             itemInstance->wasCreatedByRestrictedPlayer());
                    if (!player->mAbilities.mInstabuild)
                        itemInstance->shrink(1);
                }
            }
            else
            {
                SpawnEggItem::DisplaySpawnError(player, error);
            }
            return true;
        }
    }
    return false;
}

bool AgableMob::holdingSpawnEggOfClass(const not_null_ptr<ItemInstance>& itemInstance, unsigned long long type)
{
    if (itemInstance->getItem() == Items::spawnEgg)
    {
        unsigned long long eggType = EntityIO::sRegistry.get(SpawnEggItem::getNamedIdFrom(itemInstance));
        return type == eggType;
    }
    return false;
}

void AgableMob::defineSynchedData()
{
    Mob::defineSynchedData();
    bool baby = false;
    mEntityData->createDateItem(DATA_BABY_ID, baby);
    bool baby2 = false;
    mEntityData->createDateItem(DATA_BABY_ID_2, baby2);
}

int AgableMob::getAge()
{
    if (mLevel == NULL || mLevel->mIsClientSide)
    {
        if (mEntityData->get(DATA_BABY_ID_2))
        {
            return 1;
        }
        if (mEntityData->get(DATA_BABY_ID))
        {
            return -1;
        }
        return 0;
    }
    return mAge;
}

void AgableMob::ageUp(int amount, bool forced)
{
    int age = getAge();
    int oldAge = age;
    age += amount * 20;
    if (age > 0)
    {
        age = 0;
        if (oldAge < 0)
        {
            grewUp();
        }
    }
    int delta = age - oldAge;
    setAge(age);
    if (forced)
    {
        mForcedAge += delta;
        if (mForcedAgeTimer == 0)
        {
            mForcedAgeTimer = 40;
        }
    }
    if (getAge() == 0)
    {
        setAge(mForcedAge);
    }
}

void AgableMob::ageUp(int amount)
{
    ageUp(amount, false);
}

void AgableMob::setAge(int age)
{
    if (mLevel == NULL || !mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_BABY_ID, age < 0, true);
        mEntityData->set(DATA_BABY_ID_2, age > 0, true);
    }
    mAge = age;
    updateSize(isBaby());
}

void AgableMob::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"Age"), getAge());
    tag->putInt(std::wstring(L"ForcedAge"), mForcedAge);
}

void AgableMob::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setAge(tag->getInt(std::wstring(L"Age")));
    mForcedAge = tag->getInt(std::wstring(L"ForcedAge"));
}

void AgableMob::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor->getId() == DATA_BABY_ID->getId())
    {
        updateSize(isBaby());
    }
}

void AgableMob::aiStep()
{
    Mob::aiStep();
    if (mLevel->mIsClientSide)
    {
        if (mForcedAgeTimer > 0)
        {
            if ((mForcedAgeTimer & 3) == 0)
            {
                mLevel->addParticle(PARTICLE_HAPPY_VILLAGER, mX + (mRandom->nextFloat() * mBbWidth * 2.0f) - mBbWidth,
                                    mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                                    mZ + (mRandom->nextFloat() * mBbWidth * 2.0f) - mBbWidth, 0.0, 0.0, 0.0,
                                    arrayWithLength<int>());
            }
            --mForcedAgeTimer;
        }
    }
    else
    {
        int age = getAge();
        if (age < 0)
        {
            ++age;
            setAge(age);
            if (age == 0)
            {
                grewUp();
            }
        }
        else if (age > 0)
        {
            --age;
            setAge(age);
        }
    }
}

bool AgableMob::isBaby()
{
    return getAge() < 0;
}

unsigned int AgableMob::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::SPAWN_EGG
        && SpawnEggItem::fjGetType(holder.m_pItemInstance) == GetType())
    {
        return IDS_TOOLTIPS_PLACE;
    }
    return Mob::GetInteractTooltip(holder);
}

void AgableMob::grewUp()
{
}

void AgableMob::internalSetSize(float scale)
{
    Entity::setSize(mBaseWidth * scale, mBaseHeight * scale);
}

void AgableMob::updateSize(bool baby)
{
    internalSetSize(baby ? 0.5f : 1.0f);
}

void AgableMob::setSize(float width, float height)
{
    bool hadWidth = mBaseWidth > 0.0f;
    mBaseWidth = width;
    mBaseHeight = height;
    if (!hadWidth)
    {
        internalSetSize(1.0f);
    }
}
