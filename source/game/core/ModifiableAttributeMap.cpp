#include "game/core/ModifiableAttributeMap.hpp"

#include <vector>

#include "game/core/ModifiableAttributeInstance.hpp"

AttributeInstance* ModifiableAttributeMap::getInstance(Attribute* attribute)
{
    return BaseAttributeMap::getInstance(attribute);
}

AttributeInstance* ModifiableAttributeMap::getInstance(eATTRIBUTE_ID id)
{
    return BaseAttributeMap::getInstance(id);
}

AttributeInstance* ModifiableAttributeMap::registerAttribute(Attribute* attribute)
{
    return BaseAttributeMap::registerAttribute(attribute);
}

AttributeInstance* ModifiableAttributeMap::createAttributeInstance(Attribute* attribute)
{
    return new ModifiableAttributeInstance(this, attribute);
}

void ModifiableAttributeMap::onAttributeModified(AttributeInstance* instance)
{
    if (instance->getAttribute()->isClientSyncable())
    {
        mDirtyAttributes.insert(instance);
    }

    std::vector<Attribute*> descendants;
    getDescendants(instance->getAttribute(), descendants);

    for (std::vector<Attribute*>::iterator it = descendants.begin(); it != descendants.end(); ++it)
    {
        AttributeInstance* descendant = getInstance(*it);

        if (descendant != NULL)
        {
            static_cast<ModifiableAttributeInstance*>(descendant)->setDirty();
        }
    }
}

ModifiableAttributeMap::AttributeInstanceSet* ModifiableAttributeMap::getDirtyAttributes()
{
    return &mDirtyAttributes;
}

ModifiableAttributeMap::AttributeInstanceSet* ModifiableAttributeMap::getSyncableAttributes()
{
    AttributeInstanceSet* syncable = new AttributeInstanceSet();

    std::vector<AttributeInstance*> attributes;
    getAttributes(attributes);

    for (unsigned int i = 0; i < attributes.size(); ++i)
    {
        AttributeInstance* instance = attributes.at(i);

        if (instance->getAttribute()->isClientSyncable())
        {
            syncable->insert(instance);
        }
    }

    return syncable;
}
