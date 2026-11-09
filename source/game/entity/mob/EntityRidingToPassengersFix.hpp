#ifndef GAME_ENTITY_MOB_ENTITYRIDINGTOPASSENGERSFIX_HPP
#define GAME_ENTITY_MOB_ENTITYRIDINGTOPASSENGERSFIX_HPP

#include "game/data/DataFix.hpp"

class EntityRidingToPassengersFix : public DataFix
{
public:
    virtual ~EntityRidingToPassengersFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);

private:
    CompoundTag* extractVehicle(CompoundTag* tag);
    void addPassengerToVehicle(CompoundTag* passenger, CompoundTag* vehicle);
};

#endif
