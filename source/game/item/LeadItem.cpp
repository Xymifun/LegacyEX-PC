#include "game/item/LeadItem.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Boat.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"

LeadItem::LeadItem() : Item()
{
}

bool LeadItem::bindPlayerMobsTest(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos)
{
    double range = 7.0;
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
        typeid(Mob), AABB::newTemp(x - range, y - range, z - range, x + range, y + range, z + range));
    std::vector<boost::shared_ptr<Entity> >* boats = level->getEntitiesOfClass(
        typeid(Boat), AABB::newTemp(x - range, y - range, z - range, x + range, y + range, z + range));
    entities->insert(entities->end(), boats->begin(), boats->end());

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity->isLeashedToPlayer(player))
        {
            delete entities;
            delete boats;
            return true;
        }
    }

    delete entities;
    delete boats;
    return false;
}

bool LeadItem::bindPlayerMobs(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<LeashFenceKnotEntity> knot = LeashFenceKnotEntity::findKnotAt(level, pos);
    bool bound = false;
    double range = 7.0;
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
        typeid(Mob), AABB::newTemp(x - range, y - range, z - range, x + range, y + range, z + range));
    std::vector<boost::shared_ptr<Entity> >* boats = level->getEntitiesOfClass(
        typeid(Boat), AABB::newTemp(x - range, y - range, z - range, x + range, y + range, z + range));
    entities->insert(entities->end(), boats->begin(), boats->end());

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity->isLeashedToPlayer(player))
        {
            if (!knot)
            {
                knot = LeashFenceKnotEntity::createAndAddKnot(level, pos);
            }
            entity->setLeashedTo(knot, true);
            bound = true;
        }
    }

    delete entities;
    delete boats;
    return bound;
}

InteractionResult::EInteractionResult LeadItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                      const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                      const Direction* direction, float hitX, float hitY, float hitZ,
                                                      bool simulate)
{
    Block* block = level->getBlockState(pos)->getBlock();
    if (!block->instanceof(EBLOCK_TYPE_FENCE))
    {
        return InteractionResult::eInteractionResult_Pass;
    }

    if (simulate)
    {
        bindPlayerMobsTest(player, level, pos);
    }

    if (!level->mIsClientSide)
    {
        bindPlayerMobs(player, level, pos);
    }

    return InteractionResult::eInteractionResult_Success;
}
