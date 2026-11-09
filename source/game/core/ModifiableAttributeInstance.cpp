#include "game/core/ModifiableAttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/AttributeModifier.hpp"

typedef boost::unordered_set<AttributeModifier*> ModifierSet;

ModifiableAttributeInstance::ModifiableAttributeInstance(BaseAttributeMap* attributeMap, Attribute* attribute)
    : mDirty(true),
      mCachedValue(0.0)
{
    mAttributeMap = attributeMap;
    mAttribute = attribute;
    mBaseValue = attribute->getDefaultValue();
}

ModifiableAttributeInstance::~ModifiableAttributeInstance()
{
    for (int operation = 0; operation < NUM_OPERATIONS; ++operation)
    {
        for (ModifierSet::iterator it = mModifiersByOperation[operation].begin();
             it != mModifiersByOperation[operation].end(); ++it)
        {
            delete *it;
        }
    }
}

Attribute* ModifiableAttributeInstance::getAttribute()
{
    return mAttribute;
}

double ModifiableAttributeInstance::getBaseValue()
{
    return mBaseValue;
}

void ModifiableAttributeInstance::setDirty()
{
    mDirty = true;
    mAttributeMap->onAttributeModified(this);
}

void ModifiableAttributeInstance::setBaseValue(double baseValue)
{
    if (getBaseValue() != baseValue)
    {
        mBaseValue = baseValue;
        setDirty();
    }
}

boost::unordered_set<AttributeModifier*>* ModifiableAttributeInstance::getModifiers(int operation)
{
    return &mModifiersByOperation[operation];
}

void ModifiableAttributeInstance::getModifiers(boost::unordered_set<AttributeModifier*>& modifiers)
{
    for (int operation = 0; operation < NUM_OPERATIONS; ++operation)
    {
        ModifierSet& operationModifiers = mModifiersByOperation[operation];

        for (ModifierSet::iterator it = operationModifiers.begin(); it != operationModifiers.end(); ++it)
        {
            modifiers.insert(*it);
        }
    }
}

AttributeModifier* ModifiableAttributeInstance::getModifier(eMODIFIER_ID id)
{
    AttributeModifier* modifier = NULL;
    boost::unordered_map<unsigned int, AttributeModifier*>::iterator it = mModifiersById.find(id);

    if (it != mModifiersById.end())
    {
        modifier = it->second;
    }

    return modifier;
}

bool ModifiableAttributeInstance::hasModifier(AttributeModifier* modifier)
{
    return mModifiersById.find(modifier->getId()) != mModifiersById.end();
}

void ModifiableAttributeInstance::addModifiers(boost::unordered_set<AttributeModifier*>* modifiers)
{
    for (ModifierSet::iterator it = modifiers->begin(); it != modifiers->end(); ++it)
    {
        addModifier(*it);
    }
}

void ModifiableAttributeInstance::addModifier(AttributeModifier* modifier)
{
    if (modifier->getId() != eMODIFIER_ID_None && getModifier(modifier->getId()) != NULL)
    {
        return;
    }

    mModifiersByOperation[modifier->getOperation()].insert(modifier);
    mModifiersById[modifier->getId()] = modifier;

    setDirty();
}

void ModifiableAttributeInstance::removeModifier(AttributeModifier* modifier)
{
    ModifierSet removed;

    for (int operation = 0; operation < NUM_OPERATIONS; ++operation)
    {
        for (ModifierSet::iterator it = mModifiersByOperation[operation].begin();
             it != mModifiersByOperation[operation].end(); ++it)
        {
            if (modifier->equals(*it))
            {
                removed.insert(*it);
                mModifiersByOperation[operation].erase(it);
                break;
            }
        }
    }

    mModifiersById.erase(modifier->getId());

    setDirty();

    for (ModifierSet::iterator it = removed.begin(); it != removed.end(); ++it)
    {
        if (*it != modifier)
        {
            delete *it;
        }
    }
}

void ModifiableAttributeInstance::removeModifier(eMODIFIER_ID id)
{
    AttributeModifier* modifier = getModifier(id);

    if (modifier != NULL)
    {
        removeModifier(modifier);
        delete modifier;
    }
}

void ModifiableAttributeInstance::removeModifiers()
{
    ModifierSet modifiers;
    getModifiers(modifiers);

    for (ModifierSet::iterator it = modifiers.begin(); it != modifiers.end(); ++it)
    {
        removeModifier(*it);
    }
}

void ModifiableAttributeInstance::getAppliedModifiers(int operation,
                                                      boost::unordered_set<AttributeModifier*>& modifiers)
{
    modifiers.clear();

    ModifierSet* ownModifiers = getModifiers(operation);
    modifiers.insert(ownModifiers->begin(), ownModifiers->end());

    for (Attribute* parent = mAttribute->getParentAttribute(); parent != NULL; parent = parent->getParentAttribute())
    {
        AttributeInstance* instance = mAttributeMap->getInstance(parent);

        if (instance != NULL)
        {
            ModifierSet* parentModifiers = instance->getModifiers(operation);
            modifiers.insert(parentModifiers->begin(), parentModifiers->end());
        }
    }
}

double ModifiableAttributeInstance::calculateValue()
{
    double baseValue = getBaseValue();

    ModifierSet modifiers;

    getAppliedModifiers(0, modifiers);

    for (ModifierSet::iterator it = modifiers.begin(); it != modifiers.end(); ++it)
    {
        baseValue += (*it)->getAmount();
    }

    double value = baseValue;

    getAppliedModifiers(1, modifiers);

    for (ModifierSet::iterator it = modifiers.begin(); it != modifiers.end(); ++it)
    {
        value += baseValue * (*it)->getAmount();
    }

    getAppliedModifiers(2, modifiers);

    for (ModifierSet::iterator it = modifiers.begin(); it != modifiers.end(); ++it)
    {
        value *= 1.0 + (*it)->getAmount();
    }

    return mAttribute->sanitizeValue(value);
}

double ModifiableAttributeInstance::getValue()
{
    if (mDirty)
    {
        mCachedValue = calculateValue();
        mDirty = false;
    }

    return mCachedValue;
}
