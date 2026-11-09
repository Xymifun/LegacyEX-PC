#ifndef GAME_WORLD_BLOCK_ABSTRACTBLOCKSTATE_HPP
#define GAME_WORLD_BLOCK_ABSTRACTBLOCKSTATE_HPP 1

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/world/block/BlockState.hpp"

class Boxed;
class Property;

class AbstractBlockState : public BlockState
{
public:
    virtual const BlockState* cycle(const Property* property) const;
    virtual std::wstring toString() const;

private:
    static std::wstring PROPERTY_ENTRY_TO_STRING_FUNCTION(
        boost::unordered_map<const Property*, Boxed*>::const_iterator* entry);
    static Boxed* findNextInCollection(const Property* property, const std::vector<Boxed*>& values, Boxed* value);
};

#endif
