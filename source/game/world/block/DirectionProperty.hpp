#ifndef GAME_WORLD_BLOCK_DIRECTIONPROPERTY_HPP
#define GAME_WORLD_BLOCK_DIRECTIONPROPERTY_HPP

#include <string>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicate.hpp"

class DirectionProperty : public EnumProperty<const Direction*>
{
public:
    static DirectionProperty* create(const std::wstring& name, const boost::unordered_set<const Direction*>& values);
    static DirectionProperty* create(const std::wstring& name, const Predicate<const Direction*>* filter);
    static DirectionProperty* create(const std::wstring& name);

    static bool NOT_DOWN(const Direction* direction);


protected:
    DirectionProperty(const std::wstring& name, const boost::unordered_set<const Direction*>& values);
};

#endif
