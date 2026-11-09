#include "game/entity/mob/HangingEntity.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/DiodeBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

int gHangingEntityRenderCount = 0;

Predicate<boost::shared_ptr<Entity> >* HangingEntity::HANGING_ENTITY_SELECTOR = new HangingEntitySelector();

HangingEntity::HangingEntity(Level* level) : Entity(level, true), mPos()
{
    _init(level);
}

HangingEntity::HangingEntity(Level* level, const BlockPos& pos) : Entity(level, true), mPos()
{
    _init(level);
    mPos = pos;
}

void HangingEntity::_init(Level* level)
{
    mTickCounter = 0;
    mDirection = NULL;
    mPos = BlockPos::ZERO;
    setSize(0.5f, 0.5f);
}

void HangingEntity::defineSynchedData()
{
}

void HangingEntity::setDirection(const Direction* direction)
{
    mDirection = direction;
    mYRotO = mYRot = (float)mDirection->get2DDataValue() * 90.0f;
    recalculateBoundingBox();
}

double HangingEntity::offs(int value)
{
    return (value & 31) == 0 ? 0.5 : 0.0;
}

void HangingEntity::recalculateBoundingBox()
{
    if (mDirection == NULL)
        return;

    double x = (double)mPos.x + 0.5;
    double y = (double)mPos.y + 0.5;
    double z = (double)mPos.z + 0.5;
    double widthOffset = offs(getWidth());
    double heightOffset = offs(getHeight());
    y += heightOffset;
    x -= mDirection->getStepX() * 0.46875;
    z -= mDirection->getStepZ() * 0.46875;
    const Direction* side = mDirection->getCounterClockWise();
    x += widthOffset * side->getStepX();
    z += widthOffset * side->getStepZ();
    mX = x;
    mY = y;
    mZ = z;

    float halfWidth = (float)getWidth();
    float halfHeight = (float)getHeight();
    float halfDepth = (float)getWidth();
    if (mDirection->getAxis() == Direction::Axis::Z)
        halfDepth = 1.0f;
    else
        halfWidth = 1.0f;
    halfWidth *= 0.03125f;
    halfHeight *= 0.03125f;
    halfDepth *= 0.03125f;
    setBoundingBox(
        AABB::newTemp(x - halfWidth, y - halfHeight, z - halfDepth, x + halfWidth, y + halfHeight, z + halfDepth));
}

void HangingEntity::tick()
{
    PIXBeginNamedEvent(0.0f, "Hanging entity tick");
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    if (mTickCounter++ == 100 && !mLevel->mIsClientSide)
    {
        mTickCounter = 0;
        if (!mRemoved && !survives())
        {
            remove();
            dropItem(boost::shared_ptr<Entity>());
        }
    }
    PIXEndNamedEvent();
}

bool HangingEntity::survives()
{
    if (!mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty())
        return false;

    int width = getWidth() / 16;
    width = width < 1 ? 1 : width;
    int height = getHeight() / 16;
    height = height < 1 ? 1 : height;
    BlockPos anchor = mPos.relative(mDirection->getOpposite());
    const Direction* horizontal = mDirection->getCounterClockWise();
    MutableBlockPos check;
    int xStart = -((width - 1) / 2);
    int yStart = -((height - 1) / 2);
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            check.set(anchor)->move(horizontal, x + xStart)->move(Direction::UP, y + yStart);
            const BlockState* state = mLevel->getBlockState(check);
            if (!state->getMaterial()->isSolid() && !DiodeBlock::isDiode(state))
                return false;
        }
    }
    return mLevel->getEntities(shared_from_this(), mBoundingBox, HANGING_ENTITY_SELECTOR, NULL)->empty();
}

bool HangingEntity::isPickable()
{
    return true;
}

bool HangingEntity::skipAttackInteraction(boost::shared_ptr<Entity> entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        EntityDamageSource* source = DamageSource::CreatePlayerAttack(boost::static_pointer_cast<Player>(entity));
        bool skipped = hurt(source, 0.0f);
        delete source;
        return skipped;
    }
    return false;
}

const Direction* HangingEntity::getDirection()
{
    return mDirection;
}

bool HangingEntity::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType())
        && !boost::static_pointer_cast<Player>(attacker)->isAllowedToHurtEntity(shared_from_this()))
        return false;
    if (!mRemoved && !mLevel->mIsClientSide)
    {
        remove();
        markHurt();
        dropItem(attacker);
    }
    return true;
}

void HangingEntity::move(MoverType type, double x, double y, double z, bool pushedByPiston)
{
    if (!mLevel->mIsClientSide && !mRemoved && x * x + y * y + z * z > 0.0)
    {
        remove();
        dropItem(boost::shared_ptr<Entity>());
    }
}

void HangingEntity::push(double x, double y, double z)
{
    if (!mLevel->mIsClientSide && !mRemoved && x * x + y * y + z * z > 0.0)
    {
        remove();
        dropItem(boost::shared_ptr<Entity>());
    }
}

void HangingEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putByte(L"Facing", (unsigned char)mDirection->get2DDataValue());
    BlockPos pos = getPos();
    tag->putInt(L"TileX", pos.x);
    tag->putInt(L"TileY", pos.y);
    tag->putInt(L"TileZ", pos.z);
}

void HangingEntity::readAdditionalSaveData(CompoundTag* tag)
{
    mPos = BlockPos(tag->getInt(L"TileX"), tag->getInt(L"TileY"), tag->getInt(L"TileZ"));
    setDirection(Direction::from2DDataValue((int)tag->getByte(L"Facing")));
}

boost::shared_ptr<ItemEntity> HangingEntity::spawnAtLocation(not_null_ptr<ItemInstance> itemInstance, float yOffset)
{
    boost::shared_ptr<ItemEntity> item(
        new ItemEntity(mLevel, mX + (double)((float)mDirection->getStepX() * 0.15f), mY + static_cast<double>(yOffset),
                       mZ + (double)((float)mDirection->getStepZ() * 0.15f), itemInstance));
    item->setDefaultPickUpDelay();
    mLevel->addEntity(item);
    return item;
}

bool HangingEntity::repositionEntityAfterLoad()
{
    return false;
}

void HangingEntity::setPos(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
    BlockPos old = mPos;
    mPos = BlockPos(x, y, z);
    if (!mPos.equals(old))
    {
        recalculateBoundingBox();
        mHasImpulse = true;
    }
}

BlockPos HangingEntity::getPos()
{
    return mPos;
}

float HangingEntity::rotate(Rotation* rotation)
{
    if (mDirection != NULL && mDirection->getAxis() != Direction::Axis::Y)
    {
        switch (rotation->getType())
        {
        case Rotation::eTYPE_Clockwise90:
            mDirection = mDirection->getClockWise();
            break;
        case Rotation::eTYPE_Clockwise180:
            mDirection = mDirection->getOpposite();
            break;
        case Rotation::eTYPE_CounterClockwise90:
            mDirection = mDirection->getCounterClockWise();
            break;
        default:
            break;
        }
    }
    float result = Mth::wrapDegrees(mYRot);
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
        return result + 270.0f;
    case Rotation::eTYPE_Clockwise180:
        return result + 180.0f;
    case Rotation::eTYPE_CounterClockwise90:
        return result + 90.0f;
    default:
        return result;
    }
}

float HangingEntity::mirror(Mirror* mirror)
{
    return rotate(mirror->getRotation(mDirection));
}

void HangingEntity::thunderHit(const LightningBolt* lightningBolt)
{
}

bool HangingEntity::shouldRender(double distanceSqr)
{
    if (gHangingEntityRenderCount < 100 / gApp.GetLocalPlayerCount())
        return Entity::shouldRender(distanceSqr);
    return false;
}

bool HangingEntity::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 29584.0;
}

unsigned long long HangingEntity::GetType()
{
    return EntityType::HANGING_ENTITY;
}

bool HangingEntity::HangingEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return (entity->GetType() & EntityType::HANGING_ENTITY) == EntityType::HANGING_ENTITY;
}

HangingEntity::HangingEntitySelector::~HangingEntitySelector()
{
}
