#ifndef GAME_WORLD_GENERATION_PLAYERDATAWALKER_HPP
#define GAME_WORLD_GENERATION_PLAYERDATAWALKER_HPP

#include <string>
#include <vector>

#include "game/core/DataWalker.hpp"

class CompoundTag;
class DataFixer;

class PlayerDataWalker : public DataWalker
{
public:
    PlayerDataWalker(std::vector<std::wstring>* itemFields);
    virtual ~PlayerDataWalker();

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);

private:
    std::vector<std::wstring>* mItemFields;
};

#endif
