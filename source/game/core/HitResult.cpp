#include "game/core/HitResult.hpp"
#include <sstream>
#include "game/entity/mob/Entity.hpp"


HitResult::HitResult(Vec3* location, const Direction* direction, const BlockPos& blockPos)
    : BlockPos(),
      mType(BLOCK),
      mEntity()
{
    BlockPos::operator=(BlockPos(blockPos.x, blockPos.y, blockPos.z));
    mDirection = direction;
    mLocation = Vec3::newTemp(location->x, location->y, location->z);
    mEntity = boost::shared_ptr<Entity>();
}

HitResult::HitResult(Vec3* location, const Direction* direction)
    : BlockPos(),
      mType(BLOCK),
      mDirection(direction),
      mEntity()
{
    BlockPos::operator=(BlockPos::ZERO);
    mLocation = Vec3::newTemp(location->x, location->y, location->z);
    mEntity = boost::shared_ptr<Entity>();
}

HitResult::HitResult(boost::shared_ptr<Entity> entity) : BlockPos(), mType(ENTITY), mEntity()
{
    mEntity = entity;
    mLocation = Vec3::newTemp(entity->mX, entity->mY, entity->mZ);
}

HitResult::HitResult(Type type, Vec3* location, const Direction* direction, const BlockPos& blockPos)
    : BlockPos(),
      mType(type),
      mEntity()
{
    BlockPos::operator=(BlockPos(blockPos.x, blockPos.y, blockPos.z));
    mDirection = direction;
    mLocation = Vec3::newTemp(location->x, location->y, location->z);
    mEntity = boost::shared_ptr<Entity>();
}

HitResult::HitResult(boost::shared_ptr<Entity> entity, Vec3* location) : BlockPos(), mType(ENTITY), mEntity()
{
    mEntity = entity;
    mLocation = location;
}

const BlockPos& HitResult::getBlockPos() const
{
    return *this;
}

double HitResult::distanceTo(boost::shared_ptr<Entity> entity) const
{
    BlockPos entityBlock(entity);
    Vec3* entityLocation = Vec3::newTemp(entityBlock.x, entityBlock.y, entityBlock.z);
    return mLocation->distanceTo(entityLocation);
}

std::wstring HitResult::toString() const
{
    std::wstringstream result;
    result << L"HitResult{type=" << static_cast<int>(mType) << L", blockPos=" << x << L"," << y << L"," << z
           << L", direction=" << mDirection << L", location=" << mLocation << L", entity=" << mEntity.get() << L"}";
    return result.str();
}
