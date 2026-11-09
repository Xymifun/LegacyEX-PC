#ifndef GAME_WORLD_BLOCK_ITEMINSTANCEBLOCKENTITYTAGWALKER_HPP
#define GAME_WORLD_BLOCK_ITEMINSTANCEBLOCKENTITYTAGWALKER_HPP

#include <string>

#include "game/core/DataWalker.hpp"


class CompoundTag;
class DataFixer;

class ItemInstanceBlockEntityTagWalker : public DataWalker
{
public:
    virtual ~ItemInstanceBlockEntityTagWalker();

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);

    static void staticCtor();

private:
    std::wstring getBlockEntityId(int version, const std::wstring& itemId);
};

#endif
