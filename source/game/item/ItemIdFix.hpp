#ifndef GAME_ITEM_ITEMIDFIX_HPP
#define GAME_ITEM_ITEMIDFIX_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class ItemIdFix : public DataFix
{
public:
    virtual ~ItemIdFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);

    static void staticCtor();

private:
    static boost::unordered_map<int, std::wstring> ID_MAP;
};

#endif
