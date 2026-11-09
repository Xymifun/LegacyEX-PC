#ifndef GAME_ITEM_ITEMSPAWNEGGFIX_HPP
#define GAME_ITEM_ITEMSPAWNEGGFIX_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class ItemSpawnEggFix : public DataFix
{
public:
    ItemSpawnEggFix();

    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* item);

private:
    boost::unordered_map<int, std::wstring> mEntityNames;
};

#endif
