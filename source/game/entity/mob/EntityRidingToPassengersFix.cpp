#include "game/entity/mob/EntityRidingToPassengersFix.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"

EntityRidingToPassengersFix::~EntityRidingToPassengersFix()
{
}

int EntityRidingToPassengersFix::getVersion()
{
    return 135;
}

CompoundTag* EntityRidingToPassengersFix::extractVehicle(CompoundTag* tag)
{
    CompoundTag* vehicle = tag->getCompound(L"Riding");
    tag->remove(L"Riding");

    return vehicle;
}

void EntityRidingToPassengersFix::addPassengerToVehicle(CompoundTag* passenger, CompoundTag* vehicle)
{
    ListTag* passengers = new ListTag();
    passengers->add(passenger);
    vehicle->put(L"Passengers", passengers);
}

CompoundTag* EntityRidingToPassengersFix::fix(CompoundTag* tag)
{
    while (tag->contains(L"Riding", Tag::TAG_COMPOUND))
    {
        CompoundTag* vehicle = extractVehicle(tag);
        addPassengerToVehicle(tag, vehicle);
        tag = vehicle;
    }

    return tag;
}
