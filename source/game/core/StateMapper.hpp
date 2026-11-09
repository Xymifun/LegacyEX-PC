#ifndef GAME_CORE_STATEMAPPER_HPP
#define GAME_CORE_STATEMAPPER_HPP

#include <boost/unordered_map.hpp>

class Block;
class BlockState;
class ModelResourceLocation;

class StateMapper
{
public:
    virtual ~StateMapper() {}

    virtual boost::unordered_map<const BlockState*, ModelResourceLocation*>* mapStates(Block* block) = 0;
};

#endif
