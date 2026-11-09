#ifndef GAME_WORLD_GENERATION_ENTITYZOMBIEVILLAGERTYPEFIX_HPP
#define GAME_WORLD_GENERATION_ENTITYZOMBIEVILLAGERTYPEFIX_HPP

#include "game/data/DataFix.hpp"

class EntityZombieVillagerTypeFix : public DataFix
{
public:
    virtual ~EntityZombieVillagerTypeFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);

private:
    int getVillagerProfession(int profession);
};

#endif
