#ifndef GAME_CORE_BASEATTRIBUTEMAP_HPP
#define GAME_CORE_BASEATTRIBUTEMAP_HPP

#include <map>
#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"

class AttributeModifier;

class BaseAttributeMap
{
public:
    typedef boost::unordered_map<eATTRIBUTE_ID, AttributeInstance*> AttributeInstanceMap;
    typedef boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*> AttributeModifierMap;
    typedef std::multimap<Attribute*, Attribute*> DescendantMap;

    virtual ~BaseAttributeMap();

    virtual AttributeInstance* getInstance(Attribute* attribute);
    virtual AttributeInstance* getInstance(eATTRIBUTE_ID id);
    virtual AttributeInstance* registerAttribute(Attribute* attribute);
    virtual AttributeInstance* createAttributeInstance(Attribute* attribute) = 0;
    virtual void getAttributes(std::vector<AttributeInstance*>& attributes);
    virtual void onAttributeModified(AttributeInstance* instance);
    virtual void removeAttributeModifiers(AttributeModifierMap* modifiers);
    virtual void addAttributeModifiers(AttributeModifierMap* modifiers);

    void getDescendants(Attribute* attribute, std::vector<Attribute*>& descendants);

protected:
    AttributeInstanceMap mAttributes;
    DescendantMap mDescendantsByParent;
};

#endif
