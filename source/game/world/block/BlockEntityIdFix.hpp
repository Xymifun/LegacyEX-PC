#ifndef GAME_WORLD_BLOCK_BLOCKENTITYIDFIX_HPP
#define GAME_WORLD_BLOCK_BLOCKENTITYIDFIX_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class BlockEntityIdFix : public DataFix
{
public:
    virtual ~BlockEntityIdFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);

    static void staticCtor();

private:
    static boost::unordered_map<std::wstring, std::wstring> ID_MAP;
};

#endif
