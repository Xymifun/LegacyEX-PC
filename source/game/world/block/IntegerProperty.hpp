#ifndef GAME_WORLD_BLOCK_INTEGERPROPERTY_HPP
#define GAME_WORLD_BLOCK_INTEGERPROPERTY_HPP

#include <string>
#include <vector>

#include "game/core/Boxed.hpp"
#include "game/world/block/AbstractProperty.hpp"

class Property;

class IntegerProperty : public AbstractProperty<int>
{
public:
    IntegerProperty(const std::wstring& name, int minValue, int maxValue);
    virtual ~IntegerProperty();

    static Property* create(const std::wstring& name, int minValue, int maxValue);

    virtual const std::vector<Boxed*>& getPossibleValues() const;
    virtual bool equals(const Property* other) const;
    virtual int hashCode() const;
    virtual unsigned int getValueCount() const;
    virtual Boxed* getValueAtIndex(unsigned int index) const;
    virtual unsigned int getIndexForValue(Boxed* value) const;
    virtual std::wstring getName(const int& value) const;
    virtual int getUnboxedValue(const std::wstring& name) const;

private:
    std::vector<Boxed*> mPossibleValues;
};

#endif
