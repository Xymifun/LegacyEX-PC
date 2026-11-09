#ifndef GAME_CORE_SPAWNDATA_HPP
#define GAME_CORE_SPAWNDATA_HPP

#include "game/data/CompoundTag.hpp"
#include "game/item/WeighedRandomItem.hpp"

class SpawnData : public WeighedRandomItem
{
public:
    SpawnData();
    SpawnData(CompoundTag* tag);
    SpawnData(int weight, CompoundTag* entityTag);

    virtual ~SpawnData();

    CompoundTag* save();
    CompoundTag* getTag();
    SpawnData* copy();

private:
    CompoundTag* mTag;
};

#endif
