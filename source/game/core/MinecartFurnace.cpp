#include "game/core/MinecartFurnace.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/FurnaceBlock.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<bool>* MinecartFurnace::DATA_HAS_FUEL = NULL;

MinecartFurnace::MinecartFurnace(Level* level) : Minecart(level)
{
    fjDerivedCtorCalls();
    mFuel = 0;
    mPushX = mPushZ = 0.0;
}

MinecartFurnace::MinecartFurnace(Level* level, double x, double y, double z) : Minecart(level, x, y, z)
{
    fjDerivedCtorCalls();
    mFuel = 0;
    mPushX = mPushZ = 0.0;
}

Entity* MinecartFurnace::create(Level* level)
{
    return new MinecartFurnace(level);
}

void MinecartFurnace::staticCtor()
{
    DATA_HAS_FUEL = SynchedEntityData::defineId<bool>(EntityType::MINECART_FURNACE, EntityType::MINECART,
                                                      EntityDataSerializers::BOOLEAN);
}

unsigned int MinecartFurnace::getContainerType()
{
    return 0x11;
}

void MinecartFurnace::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
}

const Minecart::Type* MinecartFurnace::getType()
{
    return Minecart::Type::FURNACE;
}

void MinecartFurnace::defineSynchedData()
{
    Minecart::defineSynchedData();
    mEntityData->createDateItem(DATA_HAS_FUEL, false);
}

void MinecartFurnace::setHasFuel(bool hasFuelValue)
{
    mEntityData->set(DATA_HAS_FUEL, hasFuelValue, true);
}

bool MinecartFurnace::hasFuel()
{
    return mEntityData->get(DATA_HAS_FUEL);
}

void MinecartFurnace::tick()
{
    PIXBeginNamedEvent(0.0f, "Minecart furnace tick");
    Minecart::tick();
    if (mFuel > 0)
        --mFuel;
    if (mFuel <= 0)
    {
        mPushX = 0.0;
        mPushZ = 0.0;
    }
    setHasFuel(mFuel > 0);
    if (hasFuel() && mRandom->nextInt(4) == 0)
    {
        mLevel->addParticle(PARTICLE_LARGE_SMOKE, mX, mY + 0.8, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    PIXEndNamedEvent();
}

double MinecartFurnace::getMaxSpeed()
{
    return 0.2;
}

void MinecartFurnace::destroy(const DamageSource* source)
{
    Minecart::destroy(source);
    if (!source->isExplosion() && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
    {
        spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::furnace, 1)), 0.0f);
    }
}

void MinecartFurnace::moveAlongTrack(const BlockPos& pos, const BlockState* state)
{
    Minecart::moveAlongTrack(pos, state);
    double pushLength = mPushX * mPushX + mPushZ * mPushZ;
    if (pushLength > 0.0001 && mXd * mXd + mZd * mZd > 0.001)
    {
        pushLength = Mth::sqrt(pushLength);
        mPushX /= pushLength;
        mPushZ /= pushLength;
        if (mPushX * mXd + mPushZ * mZd < 0.0)
        {
            mPushX = 0.0;
            mPushZ = 0.0;
        }
        else
        {
            double scale = pushLength / getMaxSpeed();
            mPushX *= scale;
            mPushZ *= scale;
        }
    }
}

void MinecartFurnace::applyNaturalSlowdown()
{
    double pushLength = mPushX * mPushX + mPushZ * mPushZ;
    if (pushLength > 0.0001)
    {
        pushLength = Mth::sqrt(pushLength);
        mPushX /= pushLength;
        mPushZ /= pushLength;
        double friction = 0.8f;
        mXd *= friction;
        mYd *= 0.0;
        mZd *= friction;
        mXd += mPushX;
        mZd += mPushZ;
    }
    else
    {
        mXd *= 0.98f;
        mYd *= 0.0;
        mZd *= 0.98f;
    }
    Minecart::applyNaturalSlowdown();
}

bool MinecartFurnace::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return false;
    }
    if (item->getItem() == Items::coal && mFuel + 3600 <= 32000)
    {
        if (!player->mAbilities.mInstabuild)
        {
            item->shrink(1);
        }
        mFuel += 3600;
    }
    mPushX = mX - player->mX;
    mPushZ = mZ - player->mZ;
    return true;
}

unsigned int MinecartFurnace::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToInteract ? IDS_TOOLTIPS_USE : IDS_NONE;
}

void MinecartFurnace::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    tag->putDouble(L"PushX", mPushX);
    tag->putDouble(L"PushZ", mPushZ);
    tag->putShort(L"Fuel", static_cast<short>(mFuel));
}

void MinecartFurnace::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    mPushX = tag->getDouble(L"PushX");
    mPushZ = tag->getDouble(L"PushZ");
    mFuel = tag->getShort(L"Fuel");
}

const BlockState* MinecartFurnace::getDefaultDisplayBlockState()
{
    return (hasFuel() ? Blocks::lit_furnace : Blocks::furnace)
        ->defaultBlockState()
        ->setValue(FurnaceBlock::FACING, Direction::NORTH);
}

unsigned long long MinecartFurnace::GetType()
{
    return EntityType::MINECART_FURNACE;
}
