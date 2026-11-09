#ifndef GAME_WORLD_BLOCK_PROPERTY_HPP
#define GAME_WORLD_BLOCK_PROPERTY_HPP

#include <string>
#include <typeinfo>
#include <vector>

class Boxed;

class Property
{
public:
    virtual std::wstring getName() const = 0;
    virtual std::wstring getName(const Boxed* value) const = 0;
    virtual const std::vector<Boxed*>& getPossibleValues() const = 0;
    virtual const std::type_info& getValueClass() const = 0;
    virtual Boxed* getValue(const std::wstring& name) const = 0;
    virtual std::wstring toString() const = 0;
    virtual bool equals(const Property* other) const = 0;
    virtual int hashCode() const = 0;
    virtual unsigned int getValueCount() const = 0;
    virtual Boxed* getValueAtIndex(unsigned int index) const = 0;
    virtual unsigned int getIndexForValue(Boxed* value) const = 0;

    int getCachedHashCode() const;

protected:
    void updateCachedHashCode();

    int mCachedHashCode;
};

#endif
