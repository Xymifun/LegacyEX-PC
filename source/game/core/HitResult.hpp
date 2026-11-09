#ifndef GAME_CORE_HITRESULT_HPP
#define GAME_CORE_HITRESULT_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/core/Direction.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"

class Entity;

class HitResult : public BlockPos
{
public:
    enum Type
    {
        MISS,
        BLOCK,
        ENTITY
    };

    HitResult(Vec3* location, const Direction* direction, const BlockPos& blockPos);
    HitResult(Vec3* location, const Direction* direction);
    HitResult(boost::shared_ptr<Entity> entity);
    HitResult(Type type, Vec3* location, const Direction* direction, const BlockPos& blockPos);
    HitResult(boost::shared_ptr<Entity> entity, Vec3* location);

    const BlockPos& getBlockPos() const;
    double distanceTo(boost::shared_ptr<Entity> entity) const;
    std::wstring toString() const;

    Type mType;
    const Direction* mDirection;
    Vec3* mLocation;
    boost::shared_ptr<Entity> mEntity;
};

#endif
