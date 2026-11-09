#ifndef GAME_CORE_MODIFIABLEATTRIBUTEMAP_HPP
#define GAME_CORE_MODIFIABLEATTRIBUTEMAP_HPP

#include <boost/unordered_set.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"

class ModifiableAttributeMap : public BaseAttributeMap
{
public:
    typedef boost::unordered_set<AttributeInstance*> AttributeInstanceSet;


    virtual AttributeInstance* getInstance(Attribute* attribute);
    virtual AttributeInstance* getInstance(eATTRIBUTE_ID id);
    virtual AttributeInstance* registerAttribute(Attribute* attribute);
    virtual AttributeInstance* createAttributeInstance(Attribute* attribute);
    virtual void onAttributeModified(AttributeInstance* instance);
    virtual AttributeInstanceSet* getDirtyAttributes();
    virtual AttributeInstanceSet* getSyncableAttributes();

protected:
    AttributeInstanceSet mDirtyAttributes;
};

#endif
