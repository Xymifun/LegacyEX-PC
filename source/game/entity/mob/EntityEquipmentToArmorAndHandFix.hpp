#ifndef GAME_ENTITY_MOB_ENTITYEQUIPMENTTOARMORANDHANDFIX_HPP
#define GAME_ENTITY_MOB_ENTITYEQUIPMENTTOARMORANDHANDFIX_HPP

#include "game/data/DataFix.hpp"

class EntityEquipmentToArmorAndHandFix : public DataFix
{
public:
    virtual ~EntityEquipmentToArmorAndHandFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
