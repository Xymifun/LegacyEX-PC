#ifndef GAME_ENTITY_MOB_HORSECHEST_HPP
#define GAME_ENTITY_MOB_HORSECHEST_HPP

#include "game/item/SimpleContainer.hpp"

class HorseChest : public SimpleContainer
{
public:
    HorseChest(const std::wstring& name, int size);
    HorseChest(int id, const std::wstring& name, bool hasCustomName, int size);
    virtual ~HorseChest();
};

#endif
