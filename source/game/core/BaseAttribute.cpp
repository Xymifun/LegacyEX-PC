#include "game/core/BaseAttribute.hpp"

BaseAttribute::BaseAttribute(Attribute* parentAttribute, eATTRIBUTE_ID id, double defaultValue)
    : parentAttribute(parentAttribute),
      id(id),
      defaultValue(defaultValue),
      syncable(false)
{
}

eATTRIBUTE_ID BaseAttribute::getId()
{
    return this->id;
}

double BaseAttribute::getDefaultValue()
{
    return this->defaultValue;
}

bool BaseAttribute::isClientSyncable()
{
    return this->syncable;
}

BaseAttribute* BaseAttribute::setSyncable(bool syncable)
{
    this->syncable = syncable;
    return this;
}

Attribute* BaseAttribute::getParentAttribute()
{
    return this->parentAttribute;
}

bool BaseAttribute::equals(Attribute* other)
{
    return this->id == other->getId();
}
