#include "game/entity/mob/SkeletonType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/StringIds.hpp"
#include "game/client/audio/SoundEvents.hpp"

SkeletonType SkeletonType::SKELETON(0, IDS_SKELETON, &BuiltInLootTables::SKELETON);
SkeletonType SkeletonType::WITHER_SKELETON(1, IDS_WITHER, &BuiltInLootTables::WITHER_SKELETON);
SkeletonType SkeletonType::STRAY(2, IDS_SKELETON, &BuiltInLootTables::STRAY);

SkeletonType::SkeletonType(int id, int name, ResourceLocation* defaultLootTable)
    : id(id),
      name(name),
      defaultLootTable(defaultLootTable)
{
}

int SkeletonType::getId() const
{
    return id;
}

SkeletonType* SkeletonType::byId(int id)
{
    switch (id)
    {
    case 1:
        return &WITHER_SKELETON;
    case 2:
        return &STRAY;
    default:
        return &SKELETON;
    }
}

int SkeletonType::getName() const
{
    return name;
}

ResourceLocation* SkeletonType::getDefaultLootTable() const
{
    return defaultLootTable;
}

const SoundEvent* SkeletonType::getAmbientSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_AMBIENT;
    if (id == 2)
        return SoundEvents::STRAY_AMBIENT;
    return SoundEvents::SKELETON_AMBIENT;
}

const SoundEvent* SkeletonType::getHurtSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_HURT;
    if (id == 2)
        return SoundEvents::STRAY_HURT;
    return SoundEvents::SKELETON_HURT;
}

const SoundEvent* SkeletonType::getDeathSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_DEATH;
    if (id == 2)
        return SoundEvents::STRAY_DEATH;
    return SoundEvents::SKELETON_DEATH;
}

const SoundEvent* SkeletonType::getStepSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_STEP;
    if (id == 2)
        return SoundEvents::STRAY_STEP;
    return SoundEvents::SKELETON_STEP;
}
