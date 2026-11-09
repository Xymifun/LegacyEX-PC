#ifndef GAME_CORE_SALMON_HPP
#define GAME_CORE_SALMON_HPP

#include "game/core/Fish.hpp"

class CompoundTag;
class DataFixerUpper;
class EntityDataAccessor_Base;
class Level;

class Salmon : public Fish
{
public:
    enum SalmonScale
    {
        SMALL = 0,
        MEDIUM = 1,
        LARGE = 2
    };

    Salmon(Level* level);

    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);

    void setSalmonScale(SalmonScale scale);
    void SalmonWeightedRandomize();
    SalmonScale getSalmonScale();

    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void defineSynchedData();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
};

#endif
