#ifndef GAME_CORE_DEFAULTSTATEMAPPER_HPP
#define GAME_CORE_DEFAULTSTATEMAPPER_HPP

#include "game/core/AbstractStateMapper.hpp"

class BlockState;
class ModelResourceLocation;

class DefaultStateMapper : public AbstractStateMapper
{
public:
    virtual ModelResourceLocation* getLocation(const BlockState* state);
};

#endif
