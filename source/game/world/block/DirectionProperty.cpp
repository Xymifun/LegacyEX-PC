#include "game/world/block/DirectionProperty.hpp"

#include <typeinfo>

#include "game/core/Direction.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty::DirectionProperty(const std::wstring& name, const boost::unordered_set<const Direction*>& values)
    : EnumProperty<const Direction*>(name, typeid(Direction), values)
{
}

DirectionProperty* DirectionProperty::create(const std::wstring& name,
                                             const boost::unordered_set<const Direction*>& values)
{
    return new DirectionProperty(name, values);
}

DirectionProperty* DirectionProperty::create(const std::wstring& name, const Predicate<const Direction*>* filter)
{
    boost::unordered_set<const Direction*> directions(Direction::VALUES.begin(), Direction::VALUES.end());
    boost::unordered_set<const Direction*> allowedValues;

    for (boost::unordered_set<const Direction*>::const_iterator it = directions.begin(); it != directions.end(); ++it)
    {
        if (filter->apply(*it))
        {
            allowedValues.insert(*it);
        }
    }

    return create(name, allowedValues);
}

DirectionProperty* DirectionProperty::create(const std::wstring& name)
{
    return create(name, Predicates<const Direction*>::alwaysTrue);
}

bool DirectionProperty::NOT_DOWN(const Direction* direction)
{
    return direction != Direction::DOWN;
}
