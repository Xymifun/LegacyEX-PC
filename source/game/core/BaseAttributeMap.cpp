#include "game/core/BaseAttributeMap.hpp"
#include "game/core/AttributeModifier.hpp"

BaseAttributeMap::~BaseAttributeMap()
{
    for (AttributeInstanceMap::iterator it = mAttributes.begin(); it != mAttributes.end(); ++it)
    {
        delete it->second;
    }
}

AttributeInstance* BaseAttributeMap::getInstance(Attribute* attribute)
{
    return getInstance(attribute->getId());
}

AttributeInstance* BaseAttributeMap::getInstance(eATTRIBUTE_ID id)
{
    AttributeInstanceMap::iterator it = mAttributes.find(id);

    if (it != mAttributes.end())
    {
        return it->second;
    }

    return NULL;
}

AttributeInstance* BaseAttributeMap::registerAttribute(Attribute* attribute)
{
    AttributeInstanceMap::iterator it = mAttributes.find(attribute->getId());

    if (it != mAttributes.end())
    {
        return it->second;
    }

    AttributeInstance* instance = createAttributeInstance(attribute);
    mAttributes[attribute->getId()] = instance;

    Attribute* parent = attribute->getParentAttribute();
    while (parent != NULL)
    {
        mDescendantsByParent.insert(std::make_pair(parent, attribute));
        parent = parent->getParentAttribute();
    }

    return instance;
}

void BaseAttributeMap::getAttributes(std::vector<AttributeInstance*>& attributes)
{
    for (AttributeInstanceMap::iterator it = mAttributes.begin(); it != mAttributes.end(); ++it)
    {
        attributes.push_back(it->second);
    }
}

void BaseAttributeMap::onAttributeModified(AttributeInstance* instance)
{
}

void BaseAttributeMap::removeAttributeModifiers(AttributeModifierMap* modifiers)
{
    for (AttributeModifierMap::iterator it = modifiers->begin(); it != modifiers->end(); ++it)
    {
        AttributeInstance* instance = getInstance(it->first);
        AttributeModifier* modifier = it->second;

        if (instance != NULL)
        {
            instance->removeModifier(modifier);
        }

        delete modifier;
    }

    delete modifiers;
}

void BaseAttributeMap::addAttributeModifiers(AttributeModifierMap* modifiers)
{
    for (AttributeModifierMap::iterator it = modifiers->begin(); it != modifiers->end(); ++it)
    {
        AttributeInstance* instance = getInstance(it->first);
        AttributeModifier* modifier = it->second;

        if (instance != NULL)
        {
            instance->removeModifier(modifier);
            instance->addModifier(new AttributeModifier(*modifier));
        }

        delete modifier;
    }

    delete modifiers;
}

void BaseAttributeMap::getDescendants(Attribute* attribute, std::vector<Attribute*>& descendants)
{
    DescendantMap::iterator it = mDescendantsByParent.lower_bound(attribute);

    while (it != mDescendantsByParent.upper_bound(attribute))
    {
        descendants.push_back(it->second);
        ++it;
    }
}
