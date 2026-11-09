#ifndef GAME_CORE_MODIFIABLEATTRIBUTEINSTANCE_HPP
#define GAME_CORE_MODIFIABLEATTRIBUTEINSTANCE_HPP

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"

class AttributeModifier;
class BaseAttributeMap;

class ModifiableAttributeInstance : public AttributeInstance
{
public:
    enum
    {
        NUM_OPERATIONS = 3
    };

    ModifiableAttributeInstance(BaseAttributeMap* attributeMap, Attribute* attribute);
    virtual ~ModifiableAttributeInstance();

    virtual Attribute* getAttribute();
    virtual double getBaseValue();
    virtual void setBaseValue(double baseValue);
    virtual double getValue();
    virtual boost::unordered_set<AttributeModifier*>* getModifiers(int operation);
    virtual void getModifiers(boost::unordered_set<AttributeModifier*>& modifiers);
    virtual bool hasModifier(AttributeModifier* modifier);
    virtual AttributeModifier* getModifier(eMODIFIER_ID id);
    virtual void addModifiers(boost::unordered_set<AttributeModifier*>* modifiers);
    virtual void addModifier(AttributeModifier* modifier);
    virtual void removeModifier(AttributeModifier* modifier);
    virtual void removeModifier(eMODIFIER_ID id);
    virtual void removeModifiers();

    void setDirty();

private:
    void getAppliedModifiers(int operation, boost::unordered_set<AttributeModifier*>& modifiers);
    double calculateValue();

    BaseAttributeMap* mAttributeMap;
    Attribute* mAttribute;
    boost::unordered_set<AttributeModifier*> mModifiersByOperation[NUM_OPERATIONS];
    boost::unordered_map<unsigned int, AttributeModifier*> mModifiersById;
    double mBaseValue;
    bool mDirty;
    double mCachedValue;
};

#endif
