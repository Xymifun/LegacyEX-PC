#ifndef GAME_ENTITY_MOB_ENTITYPAINTINGITEMFRAMEDIRECTIONFIX_HPP
#define GAME_ENTITY_MOB_ENTITYPAINTINGITEMFRAMEDIRECTIONFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityPaintingItemFrameDirectionFix : public DataFix
{
public:
    virtual ~EntityPaintingItemFrameDirectionFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
