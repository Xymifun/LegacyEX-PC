#ifndef GAME_WORLD_BLOCK_BOOLEANPROPERTY_HPP
#define GAME_WORLD_BLOCK_BOOLEANPROPERTY_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/Boxed.hpp"
#include "game/world/block/AbstractProperty.hpp"

class Property;

class BooleanProperty : public AbstractProperty<bool>
{
public:
    BooleanProperty(const std::wstring& name);
    virtual ~BooleanProperty();

    static Property* create(const std::wstring& name);

    virtual const std::vector<Boxed*>& getPossibleValues() const;
    virtual bool equals(const Property* other) const;
    virtual int hashCode() const;
    virtual unsigned int getValueCount() const;
    virtual Boxed* getValueAtIndex(unsigned int index) const;
    virtual unsigned int getIndexForValue(Boxed* value) const;
    virtual std::wstring getName(const bool& value) const;
    virtual bool getUnboxedValue(const std::wstring& name) const;

private:
    std::vector<Boxed*> mPossibleValues;
    boost::unordered_map<std::wstring, bool> mValuesByName;
};

#endif
