#ifndef GAME_CORE_BASEATTRIBUTE_HPP
#define GAME_CORE_BASEATTRIBUTE_HPP

#include "game/core/Attribute.hpp"

class BaseAttribute : public Attribute
{
public:
    BaseAttribute(Attribute* parentAttribute, eATTRIBUTE_ID id, double defaultValue);
    virtual ~BaseAttribute() {}

    virtual eATTRIBUTE_ID getId();
    virtual double getDefaultValue();
    virtual bool isClientSyncable();
    virtual Attribute* getParentAttribute();
    virtual BaseAttribute* setSyncable(bool syncable);

    bool equals(Attribute* other);

protected:
    Attribute* parentAttribute;
    eATTRIBUTE_ID id;
    double defaultValue;
    bool syncable;
};

#endif
