#ifndef GAME_CORE_ATTRIBUTEINSTANCE_HPP
#define GAME_CORE_ATTRIBUTEINSTANCE_HPP

#include <boost/unordered_set.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/AttributeModifier.hpp"

class AttributeInstance
{
public:
    virtual ~AttributeInstance() {}

    virtual Attribute* getAttribute() = 0;
    virtual double getBaseValue() = 0;
    virtual void setBaseValue(double baseValue) = 0;
    virtual double getValue() = 0;
    virtual boost::unordered_set<AttributeModifier*>* getModifiers(int operation) = 0;
    virtual void getModifiers(boost::unordered_set<AttributeModifier*>& modifiers) = 0;
    virtual bool hasModifier(AttributeModifier* modifier) = 0;
    virtual AttributeModifier* getModifier(eMODIFIER_ID id) = 0;
    virtual void addModifiers(boost::unordered_set<AttributeModifier*>* modifiers) = 0;
    virtual void addModifier(AttributeModifier* modifier) = 0;
    virtual void removeModifier(AttributeModifier* modifier) = 0;
    virtual void removeModifier(eMODIFIER_ID id) = 0;
    virtual void removeModifiers() = 0;
};

#endif
