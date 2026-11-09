#include "game/entity/mob/ZombieType.hpp"
#include "game/client/audio/SoundEvents.hpp"

static ZombieType normalZombieType(0, 0, false);
static ZombieType villagerZombieType(1, 1, true);
static ZombieType huskZombieType(2, 2, false);
static ZombieType drownedZombieType(3, 3, false);

ZombieType::ZombieType(int id, int name, bool villagerZombie) : id(id), villagerZombie(villagerZombie), name(name)
{
}

int ZombieType::getId() const
{
    return id;
}

bool ZombieType::isVillagerZombie() const
{
    return villagerZombie;
}

int ZombieType::getVillagerProfession() const
{
    if (!villagerZombie)
        return -1;
    return id;
}

ZombieType* ZombieType::byId(int id)
{
    switch (id)
    {
    case 0:
        return &normalZombieType;
    case 1:
        return &villagerZombieType;
    case 2:
        return &huskZombieType;
    case 3:
        return &drownedZombieType;
    default:
        return 0;
    }
}

ZombieType* ZombieType::byVillagerProffesion(int profession)
{
    if (profession < 0 || profession > 5)
        return 0;
    return &villagerZombieType;
}

int ZombieType::getName() const
{
    return name;
}

bool ZombieType::isSunSensitive() const
{
    return id != 2 && id != 3;
}

const SoundEvent* ZombieType::getAmbientSound() const
{
    if (id == 2)
        return SoundEvents::HUSK_AMBIENT;
    if (id == 3)
        return SoundEvents::DROWNED_AMBIENT;
    return SoundEvents::ZOMBIE_AMBIENT;
}

const SoundEvent* ZombieType::getHurtSound() const
{
    if (id == 2)
        return SoundEvents::HUSK_HURT;
    if (id == 3)
        return SoundEvents::DROWNED_HURT;
    return SoundEvents::ZOMBIE_HURT;
}

const SoundEvent* ZombieType::getDeathSound() const
{
    if (id == 2)
        return SoundEvents::HUSK_DEATH;
    if (id == 3)
        return SoundEvents::DROWNED_DEATH;
    return SoundEvents::ZOMBIE_DEATH;
}

const SoundEvent* ZombieType::getStepSound() const
{
    if (id == 2)
        return SoundEvents::HUSK_STEP;
    if (id == 3)
        return SoundEvents::DROWNED_STEP;
    return SoundEvents::ZOMBIE_STEP;
}
