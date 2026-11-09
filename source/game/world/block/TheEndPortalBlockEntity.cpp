#include "game/world/block/TheEndPortalBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Direction.hpp"

BlockEntity* TheEndPortalBlockEntity::create()
{
    return new TheEndPortalBlockEntity();
}

boost::shared_ptr<BlockEntity> TheEndPortalBlockEntity::clone()
{
    boost::shared_ptr<TheEndPortalBlockEntity> result(new TheEndPortalBlockEntity());
    BlockEntity::clone(result);
    return result;
}

bool TheEndPortalBlockEntity::shouldRenderFace(const Direction* direction)
{
    return direction == Direction::UP;
}

unsigned long long TheEndPortalBlockEntity::GetType()
{
    return BlockEntityType::THE_END_PORTAL;
}
