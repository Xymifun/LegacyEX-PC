#ifndef GAME_CORE_TROPICALFISH_HPP
#define GAME_CORE_TROPICALFISH_HPP

#include <string>

#include "game/core/Fish.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CompoundTag;
class DataFixerUpper;
class DifficultyInstance;
template <class T> class EntityDataAccessor;

class TropicalFish : public Fish
{
public:
    class TropicalFishGroupData : public MobGroupData
    {
    public:
        TropicalFishGroupData(int baseVariant, int markVariant, int color1, int color2);
        virtual ~TropicalFishGroupData();

        int mBaseVariant;
        int mMarkVariant;
        int mColor1;
        int mColor2;
    };

    TropicalFish(Level* level);
    virtual ~TropicalFish();

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();

    virtual void tick();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();

    int getVariant();
    int getBaseVariant();
    int getMarkVariant();
    void setVariant(int variant);
    int getColor();
    int getColor1();
    int getColor2();
    void setColor(int color);
    void clearLayeredTextureInfo();
    void rebuildLayeredTextureInfo();
    std::wstring getLayeredTextureHashName();
    arrayWithLength<int> getLayeredTextureLayers();

private:
    static const EntityDataAccessor<int>* DATA_VARIANT;
    static const EntityDataAccessor<int>* DATA_COLOR;

    std::wstring mLayeredTextureHashName;
    arrayWithLength<int> mLayeredTextureLayers;
};

#endif
