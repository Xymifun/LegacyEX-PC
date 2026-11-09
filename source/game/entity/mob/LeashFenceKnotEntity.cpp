#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Boat.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/LeadItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

LeashFenceKnotEntity::LeashFenceKnotEntity(Level* level) : HangingEntity(level)
{
    _init();
}

LeashFenceKnotEntity::LeashFenceKnotEntity(Level* level, const BlockPos& pos) : HangingEntity(level, pos)
{
    _init();
    setPos((double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5);
    float yOffset = 0.125f;
    float halfWidth = 0.1875f;
    float halfHeight = 0.25f;
    setBoundingBox(AABB::newPermanent(mX - halfWidth, mY - halfHeight + yOffset, mZ - halfWidth, mX + halfWidth,
                                      mY + halfHeight + yOffset, mZ + halfWidth));
}

Entity* LeashFenceKnotEntity::create(Level* level)
{
    return new LeashFenceKnotEntity(level);
}

void LeashFenceKnotEntity::_init()
{
    defineSynchedData();
    fjDerivedCtorCalls();
}

void LeashFenceKnotEntity::setPos(double x, double y, double z)
{
    HangingEntity::setPos(std::floor(x) + 0.5, std::floor(y) + 0.5, std::floor(z) + 0.5);
}

void LeashFenceKnotEntity::defineSynchedData()
{
    HangingEntity::defineSynchedData();
}

void LeashFenceKnotEntity::recalculateBoundingBox()
{
    mX = (double)mPos.x + 0.5;
    mY = (double)mPos.y + 0.5;
    mZ = (double)mPos.z + 0.5;
}

void LeashFenceKnotEntity::setDirection(const Direction* direction)
{
}

int LeashFenceKnotEntity::getWidth()
{
    return 9;
}

int LeashFenceKnotEntity::getHeight()
{
    return 9;
}

float LeashFenceKnotEntity::getEyeHeight()
{
    return -0.0625f;
}

bool LeashFenceKnotEntity::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 1024.0;
}

void LeashFenceKnotEntity::dropItem(boost::shared_ptr<Entity> breaker)
{
    playSound(SoundEvents::LEASHKNOT_BREAK, 1.0f, 1.0f);
}

void LeashFenceKnotEntity::playPlacementSound()
{
    playSound(SoundEvents::LEASHKNOT_PLACE, 1.0f, 1.0f);
}

bool LeashFenceKnotEntity::save(CompoundTag* tag)
{
    return false;
}

void LeashFenceKnotEntity::addAdditonalSaveData(CompoundTag* tag)
{
}

void LeashFenceKnotEntity::readAdditionalSaveData(CompoundTag* tag)
{
}

bool LeashFenceKnotEntity::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (mLevel->mIsClientSide)
        return true;

    bool changed = false;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(
        typeid(Mob), AABB::newTemp(mX - 7.0, mY - 7.0, mZ - 7.0, mX + 7.0, mY + 7.0, mZ + 7.0));
    std::vector<boost::shared_ptr<Entity> >* boats = mLevel->getEntitiesOfClass(
        typeid(Boat), AABB::newTemp(mX - 7.0, mY - 7.0, mZ - 7.0, mX + 7.0, mY + 7.0, mZ + 7.0));
    entities->insert(entities->end(), boats->begin(), boats->end());

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity->isLeashedToPlayer(player))
        {
            entity->setLeashedTo(shared_from_this(), true);
            changed = true;
        }
    }

    if (!changed && player->isAllowedToMine())
    {
        remove();
        if (player->mAbilities.mInstabuild)
        {
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
            {
                boost::shared_ptr<Entity> entity = *it;
                if (entity->isLeashed() && entity->getLeashHolder() == shared_from_this())
                    entity->dropLeash(true, false);
            }
        }
    }
    delete entities;
    delete boats;
    return true;
}

bool LeashFenceKnotEntity::survives()
{
    return dynamic_cast<FenceBlock*>(mLevel->getBlockState(mPos)->getBlock()) != NULL;
}

boost::shared_ptr<LeashFenceKnotEntity> LeashFenceKnotEntity::createAndAddKnot(Level* level, const BlockPos& pos)
{
    boost::shared_ptr<LeashFenceKnotEntity> knot(new LeashFenceKnotEntity(level, pos));
    level->addEntity(knot);
    knot->playPlacementSound();
    return knot;
}

boost::shared_ptr<LeashFenceKnotEntity> LeashFenceKnotEntity::findKnotAt(Level* level, const BlockPos& pos)
{
    AABB* bounds = AABB::newTemp((double)pos.x - 1.0, (double)pos.y - 1.0, (double)pos.z - 1.0, (double)pos.x + 1.0,
                                 (double)pos.y + 1.0, (double)pos.z + 1.0);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(LeashFenceKnotEntity), bounds);
    if (entities != NULL)
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<LeashFenceKnotEntity> knot = boost::dynamic_pointer_cast<LeashFenceKnotEntity>(*it);
            if (knot->getPos().equals(pos))
            {
                delete entities;
                return knot;
            }
        }
        delete entities;
    }
    return boost::shared_ptr<LeashFenceKnotEntity>();
}

unsigned int LeashFenceKnotEntity::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    int bound = Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::LEAD
                && LeadItem::bindPlayerMobsTest(tooltipData.m_pPlayer, mLevel, BlockPos(tooltipData.m_pPlayer));
    return bound ? IDS_TOOLTIPS_ATTACH : IDS_TOOLTIPS_UNLEASH;
}

unsigned int LeashFenceKnotEntity::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return IDS_TOOLTIPS_UNLEASH;
}

unsigned long long LeashFenceKnotEntity::GetType()
{
    return EntityType::LEASH_FENCE_KNOT_ENTITY;
}
