#ifndef GAME_ENTITY_MOB_SKELETONTYPE_HPP
#define GAME_ENTITY_MOB_SKELETONTYPE_HPP

#include "game/util/ResourceLocation.hpp"

class SoundEvent;

class SkeletonType
{
public:
    static SkeletonType SKELETON;
    static SkeletonType WITHER_SKELETON;
    static SkeletonType STRAY;

    SkeletonType(int id, int name, ResourceLocation* defaultLootTable);

    int getId() const;
    int getName() const;

    ResourceLocation* getDefaultLootTable() const;

    const SoundEvent* getAmbientSound() const;
    const SoundEvent* getHurtSound() const;
    const SoundEvent* getDeathSound() const;
    const SoundEvent* getStepSound() const;

    static SkeletonType* byId(int id);

private:
    int id;
    int name;
    ResourceLocation* defaultLootTable;
};

#endif
