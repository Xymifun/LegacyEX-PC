#ifndef GAME_CORE_LIGHTNINGBOLT_HPP
#define GAME_CORE_LIGHTNINGBOLT_HPP

#include <vector>

#include "game/client/audio/SoundSource.hpp"
#include "game/entity/mob/GlobalEntity.hpp"

class CompoundTag;
class Level;

class LightningBolt : public GlobalEntity
{
public:
    LightningBolt(Level* level, double x, double y, double z, bool visualOnly);

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual bool shouldRender(double distanceSqr);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual SoundSource::ESoundSource getSoundSource();

    int mLife;
    std::vector<int> mHitEntities;
    long long mSeed;
    int mFlashes;
    bool mVisualOnly;
};

#endif
