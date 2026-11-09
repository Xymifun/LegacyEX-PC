#include "game/core/EnderCrystal.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/core/DamageSource.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/NbtUtils.hpp"
#include "game/util/Random.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

const EntityDataAccessor<BlockPos*>* EnderCrystal::DATA_BEAM_TARGET = NULL;
const EntityDataAccessor<bool>* EnderCrystal::DATA_SHOW_BOTTOM = NULL;

EnderCrystal::EnderCrystal(Level* level) : Entity(level, true)
{
    _init(level);
}

EnderCrystal::EnderCrystal(Level* level, double x, double y, double z) : Entity(level, true)
{
    _init(level);
    setPos(x, y, z);
}

void EnderCrystal::staticCtor()
{
    DATA_BEAM_TARGET = SynchedEntityData::defineId(EntityType::ENDER_CRYSTAL, EntityType::ENTITY,
                                                   EntityDataSerializers::OPTIONAL_BLOCK_POS);
    DATA_SHOW_BOTTOM = SynchedEntityData::defineId(EntityType::ENDER_CRYSTAL, EntityType::ENTITY,
                                                   EntityDataSerializers::BOOLEAN);
}

void EnderCrystal::_init(Level* level)
{
    fjDerivedCtorCalls();
    mBlocksBuilding = true;
    setSize(2.0f, 2.0f);
    mTime = mRandom->nextInt(100000);
}

bool EnderCrystal::makeStepSound()
{
    return false;
}

void EnderCrystal::defineSynchedData()
{
    getEntityData()->define(DATA_BEAM_TARGET, static_cast<BlockPos*>(NULL));
    getEntityData()->define(DATA_SHOW_BOTTOM, true);
}

void EnderCrystal::tick()
{
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    ++mTime;

    if (!mLevel->mIsClientSide)
    {
        BlockPos pos(this);
        if (mLevel->mDimension->getType() == DimensionType::THE_END
            && mLevel->getBlockState(pos)->getBlock() != Blocks::fire)
        {
            mLevel->setBlockAndUpdate(pos, Blocks::fire->defaultBlockState());
        }
    }
}

BlockPos* EnderCrystal::getBeamTarget()
{
    return getEntityData()->get(DATA_BEAM_TARGET);
}

bool EnderCrystal::showsBottom()
{
    return getEntityData()->get(DATA_SHOW_BOTTOM);
}

void EnderCrystal::addAdditonalSaveData(CompoundTag* tag)
{
    if (getBeamTarget() != 0)
        tag->put(L"BeamTarget", NbtUtils::writeBlockPos(*getBeamTarget()));
    tag->putBoolean(L"ShowBottom", showsBottom());
}

void EnderCrystal::setBeamTarget(BlockPos* pos)
{
    getEntityData()->set(DATA_BEAM_TARGET, pos, true);
}

void EnderCrystal::setShowBottom(bool showBottom)
{
    getEntityData()->set(DATA_SHOW_BOTTOM, showBottom, true);
}

void EnderCrystal::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"BeamTarget", Tag::TAG_COMPOUND))
        setBeamTarget(new BlockPos(NbtUtils::readBlockPos(tag->getCompound(L"BeamTarget"))));
    if (tag->contains(L"ShowBottom", Tag::TAG_BYTE))
        setShowBottom(tag->getBoolean(L"ShowBottom"));
}

bool EnderCrystal::isPickable()
{
    return true;
}

void EnderCrystal::onDestroyedBy(const DamageSource* source)
{
    TheEndDimension* dimension = dynamic_cast<TheEndDimension*>(mLevel->mDimension);
    if (dimension != NULL)
    {
        EndDragonFight* fight = dimension->getDragonFight();
        if (fight != NULL)
            fight->onCrystalDestroyed(boost::static_pointer_cast<EnderCrystal>(shared_from_this()), source);
    }
}

bool EnderCrystal::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    if (source->getEntity() && eTYPE_DERIVED_FROM(EntityType::ENDER_DRAGON, source->getEntity()->GetType()))
        return false;

    if (!mRemoved && !mLevel->mIsClientSide)
    {
        remove();
        if (!mLevel->mIsClientSide)
        {
            if (gApp.GetGameHostOption(eGameHostOption_TNT) && mLevel->explodeNextEnderCrystal()
                && !source->isExplosion())
                mLevel->explode(boost::shared_ptr<Entity>(), mX, mY, mZ, 6.0f, false, true, false, false, false, -1.0f);
            onDestroyedBy(source);
        }
    }
    return true;
}

void EnderCrystal::kill()
{
    onDestroyedBy(DamageSource::GENERIC);
    Entity::kill();
}

bool EnderCrystal::shouldRenderAtSqrDistance(double distanceSqr)
{
    return Entity::shouldRenderAtSqrDistance(distanceSqr) || getBeamTarget() != NULL;
}

unsigned int EnderCrystal::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return IDS_NONE;
}

unsigned long long EnderCrystal::GetType()
{
    return EntityType::ENDER_CRYSTAL;
}

Entity* EnderCrystal::create(Level* level)
{
    return new EnderCrystal(level);
}
