#ifndef GAME_ENTITY_MOB_ZOMBIETYPE_HPP
#define GAME_ENTITY_MOB_ZOMBIETYPE_HPP

#include "game/client/audio/SoundEvent.hpp"

class ZombieType
{
public:
    ZombieType(int id, int name, bool villagerZombie);

    int getId() const;
    bool isVillagerZombie() const;
    int getVillagerProfession() const;
    static ZombieType* byId(int id);
    static ZombieType* byVillagerProffesion(int profession);
    int getName() const;
    bool isSunSensitive() const;
    const SoundEvent* getAmbientSound() const;
    const SoundEvent* getHurtSound() const;
    const SoundEvent* getDeathSound() const;
    const SoundEvent* getStepSound() const;

private:
    int id;
    bool villagerZombie;
    int name;
};

#endif
