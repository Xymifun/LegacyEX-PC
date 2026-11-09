#ifndef GAME_ITEM_ITEMSTAT_HPP
#define GAME_ITEM_ITEMSTAT_HPP

#include <string>
#include <stdint.h>

#include "game/core/Stat.hpp"

class ItemStat : public Stat
{
public:
    ItemStat(int id, const std::wstring& name, int itemId);

    uint32_t getItemId();

    ItemStat* setExtended();

protected:
    int itemId;
};

#endif
