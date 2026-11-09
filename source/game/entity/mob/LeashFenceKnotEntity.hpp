#ifndef GAME_ENTITY_MOB_LEASHFENCEKNOTENTITY_HPP
#define GAME_ENTITY_MOB_LEASHFENCEKNOTENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/HangingEntity.hpp"

class EntityTooltipDataHolder;

class LeashFenceKnotEntity : public HangingEntity
{
public:
    LeashFenceKnotEntity(Level* level);
    LeashFenceKnotEntity(Level* level, const BlockPos& pos);
    virtual unsigned long long GetType();

    virtual void defineSynchedData();
    virtual void setPos(double x, double y, double z);
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual bool save(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual float getEyeHeight();
    virtual void setDirection(const Direction* direction);
    virtual void recalculateBoundingBox();
    virtual bool survives();
    virtual int getWidth();
    virtual int getHeight();
    virtual void dropItem(boost::shared_ptr<Entity> breaker);
    virtual void playPlacementSound();

    static Entity* create(Level* level);
    void _init();
    static boost::shared_ptr<LeashFenceKnotEntity> createAndAddKnot(Level* level, const BlockPos& pos);
    static boost::shared_ptr<LeashFenceKnotEntity> findKnotAt(Level* level, const BlockPos& pos);
};

#endif
