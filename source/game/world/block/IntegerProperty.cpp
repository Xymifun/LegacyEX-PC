#include "game/world/block/IntegerProperty.hpp"

#include "game/core/Boxed.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

template <typename T>
std::wstring _toString(T value);

template <typename T>
T _fromString(const std::wstring& text);

IntegerProperty::IntegerProperty(const std::wstring& name, int minValue, int maxValue)
    : AbstractProperty<int>(name, typeid(int))
{
    for (int value = minValue; value <= maxValue; ++value)
    {
        mPossibleValues.push_back(new TypedBoxed<int>(value));
    }

    updateCachedHashCode();
}

IntegerProperty::~IntegerProperty()
{
    for (std::vector<Boxed*>::iterator it = mPossibleValues.begin(); it != mPossibleValues.end(); ++it)
    {
        delete *it;
    }
}

const std::vector<Boxed*>& IntegerProperty::getPossibleValues() const
{
    return mPossibleValues;
}

bool IntegerProperty::equals(const Property* other) const
{
    if (this == other)
    {
        return true;
    }

    const IntegerProperty* property = dynamic_cast<const IntegerProperty*>(other);
    if (property == NULL)
    {
        return false;
    }

    if (!AbstractProperty<int>::equals(other))
    {
        return false;
    }

    return mPossibleValues == property->mPossibleValues;
}

int IntegerProperty::hashCode() const
{
    return (((int)(std::size_t)mValueClass * 31) + javaStringHashCode(mName)) * 31 + hashBoxedSet(mPossibleValues);
}

Property* IntegerProperty::create(const std::wstring& name, int minValue, int maxValue)
{
    return new IntegerProperty(name, minValue, maxValue);
}

std::wstring IntegerProperty::getName(const int& value) const
{
    return _toString(value);
}

int IntegerProperty::getUnboxedValue(const std::wstring& name) const
{
    return _fromString<int>(name);
}

unsigned int IntegerProperty::getValueCount() const
{
    return (unsigned int)mPossibleValues.size();
}

Boxed* IntegerProperty::getValueAtIndex(unsigned int index) const
{
    return mPossibleValues.at(index);
}

unsigned int IntegerProperty::getIndexForValue(Boxed* value) const
{
    unsigned int index = 0;
    for (std::vector<Boxed*>::const_iterator it = mPossibleValues.begin(); it != mPossibleValues.end(); ++it)
    {
        if (*value == *it)
        {
            return index;
        }
        ++index;
    }
    return 0;
}
