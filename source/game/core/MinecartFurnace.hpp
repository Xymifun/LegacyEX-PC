#ifndef GAME_CORE_MINECARTFURNACE_HPP
#define GAME_CORE_MINECARTFURNACE_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/Minecart.hpp"

class BlockPos;
class BlockState;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class Player;
template <class T> class EntityDataAccessor;

class MinecartFurnace : public Minecart
{
public:
    MinecartFurnace(Level* level);
    MinecartFurnace(Level* level, double x, double y, double z);
    static Entity* create(Level* level);
    static void staticCtor();
    virtual unsigned int getContainerType();
    static void addDataWalkers(DataFixerUpper* dataFixerUpper);
    virtual const Minecart::Type* getType();
    virtual void defineSynchedData();
    void setHasFuel(bool hasFuel);
    bool hasFuel();
    virtual void tick();
    virtual double getMaxSpeed();
    virtual void destroy(const DamageSource* source);
    virtual void moveAlongTrack(const BlockPos& pos, const BlockState* state);
    virtual void applyNaturalSlowdown();
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual const BlockState* getDefaultDisplayBlockState();
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<bool>* DATA_HAS_FUEL;
    int mFuel;
    double mPushX;
    double mPushZ;
};

#endif
