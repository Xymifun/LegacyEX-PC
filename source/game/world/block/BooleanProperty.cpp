#include "game/world/block/BooleanProperty.hpp"

#include "game/core/Boxed.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <string>
#include <typeinfo>
#include <vector>

BooleanProperty::BooleanProperty(const std::wstring& name) : AbstractProperty<bool>(name, typeid(bool))
{
    mValuesByName[L"true"] = true;
    mValuesByName[L"false"] = false;

    mPossibleValues.push_back(new TypedBoxed<bool>(true));
    mPossibleValues.push_back(new TypedBoxed<bool>(false));

    updateCachedHashCode();
}

BooleanProperty::~BooleanProperty()
{
    for (std::vector<Boxed*>::iterator it = mPossibleValues.begin(); it != mPossibleValues.end(); ++it)
    {
        delete *it;
    }
}

Property* BooleanProperty::create(const std::wstring& name)
{
    return new BooleanProperty(name);
}

const std::vector<Boxed*>& BooleanProperty::getPossibleValues() const
{
    return mPossibleValues;
}

bool BooleanProperty::equals(const Property* other) const
{
    if (this == other)
    {
        return true;
    }

    const BooleanProperty* property = dynamic_cast<const BooleanProperty*>(other);
    if (property == NULL)
    {
        return false;
    }

    if (!AbstractProperty<bool>::equals(other))
    {
        return false;
    }

    return mPossibleValues == property->mPossibleValues;
}

int BooleanProperty::hashCode() const
{
    return ((int)(intptr_t)mValueClass * 31 + javaStringHashCode(mName)) * 31 + hashBoxedSet(mPossibleValues);
}

unsigned int BooleanProperty::getValueCount() const
{
    return 2;
}

Boxed* BooleanProperty::getValueAtIndex(unsigned int index) const
{
    return getPossibleValues().at(index);
}

unsigned int BooleanProperty::getIndexForValue(Boxed* value) const
{
    unsigned int index = 0;

    const std::vector<Boxed*>& possibleValues = getPossibleValues();
    for (std::vector<Boxed*>::const_iterator it = possibleValues.begin(); it != possibleValues.end(); ++it)
    {
        Boxed* possibleValue = *it;
        if (*value == possibleValue)
        {
            return index;
        }

        ++index;
    }

    return 0;
}

std::wstring BooleanProperty::getName(const bool& value) const
{
    return _toString(value);
}

bool BooleanProperty::getUnboxedValue(const std::wstring& name) const
{
    boost::unordered_map<std::wstring, bool>::const_iterator it = mValuesByName.find(name);
    if (it != mValuesByName.end())
    {
        return it->second;
    }

    return false;
}
