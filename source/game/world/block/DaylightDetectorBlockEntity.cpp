#include "game/world/block/DaylightDetectorBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/Level.hpp"
#include "game/world/block/DaylightDetectorBlock.hpp"

DaylightDetectorBlockEntity::DaylightDetectorBlockEntity() : BlockEntity()
{
}

BlockEntity* DaylightDetectorBlockEntity::create()
{
    return new DaylightDetectorBlockEntity();
}

boost::shared_ptr<BlockEntity> DaylightDetectorBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new DaylightDetectorBlockEntity());
    BlockEntity::clone(result);
    return result;
}

void DaylightDetectorBlockEntity::tick()
{
    if (mLevel != NULL && !mLevel->mIsClientSide && mLevel->getGameTime() % 20LL == 0LL)
    {
        mBlock = getBlock();
        if (dynamic_cast<DaylightDetectorBlock*>(mBlock) != NULL)
        {
            static_cast<DaylightDetectorBlock*>(mBlock)->updateSignalStrength(mLevel, mPos);
        }
    }
}

unsigned long long DaylightDetectorBlockEntity::GetType()
{
    return BlockEntityType::DAYLIGHT_DETECTOR;
}
