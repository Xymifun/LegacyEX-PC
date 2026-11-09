#ifndef GAME_DATA_DATAFIXER_HPP
#define GAME_DATA_DATAFIXER_HPP

#include "game/data/DataFixTypes.hpp"

class CompoundTag;

class DataFixer
{
public:
    virtual CompoundTag* update(DataFixTypes::EDataFixTypes type, CompoundTag* tag, int version) = 0;
};

#endif
