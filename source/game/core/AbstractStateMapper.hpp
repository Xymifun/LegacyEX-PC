#ifndef GAME_CORE_ABSTRACTSTATEMAPPER_HPP
#define GAME_CORE_ABSTRACTSTATEMAPPER_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/StateMapper.hpp"

class Block;
class BlockState;
class Boxed;
class ModelResourceLocation;
class Property;

class AbstractStateMapper : public StateMapper
{
public:
    virtual ~AbstractStateMapper() {}

    virtual boost::unordered_map<const BlockState*, ModelResourceLocation*>* mapStates(Block* block);
    virtual ModelResourceLocation* getLocation(const BlockState* state) = 0;

protected:
    std::wstring getValuesString(const boost::unordered_map<const Property*, Boxed*>* values);

    boost::unordered_map<const BlockState*, ModelResourceLocation*> mStateLocations;
};

#endif
