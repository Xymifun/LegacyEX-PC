#ifndef GAME_WORLD_BLOCK_ABSTRACTPROPERTY_HPP
#define GAME_WORLD_BLOCK_ABSTRACTPROPERTY_HPP

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>
#include <boost/functional/hash.hpp>
#include <boost/unordered_set.hpp>
#include "game/core/Boxed.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

int javaStringHashCode(const std::wstring& value);

template <class T>
class AbstractProperty : public Property
{
public:
    virtual std::wstring getName() const { return mName; }
    virtual std::wstring getName(const Boxed* value) const
    {
        return getName((value->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(value) : NULL)->getValue());
    }
    virtual const std::type_info& getValueClass() const { return *mValueClass; }
    virtual Boxed* getValue(const std::wstring& name) const { return new TypedBoxed<T>(getUnboxedValue(name)); }
    virtual std::wstring toString() const { return std::wstring(L""); }
    virtual bool equals(const Property* other) const
    {
        if (this == other)
        {
            return true;
        }
        const AbstractProperty<T>* property = dynamic_cast<const AbstractProperty<T>*>(other);
        if (property != NULL)
        {
            return mValueClass == property->mValueClass && mName == property->mName;
        }
        return false;
    }
    virtual int hashCode() const { return (int)((std::size_t)mValueClass * 31U) + javaStringHashCode(mName); }
    virtual std::wstring getName(const T& value) const = 0;
    virtual T getUnboxedValue(const std::wstring& name) const = 0;
    void getUnboxedPossibleValues(boost::unordered_set<T>& values) const;

protected:
    AbstractProperty(const std::wstring& name, const std::type_info& valueClass) : mValueClass(&valueClass), mName(name)
    {
    }
    static int hashBoxedSet(const std::vector<Boxed*>& values)
    {
        int hash = 0;
        static boost::hash<T> hasher = boost::hash<T>();
        typename std::vector<Boxed*>::const_iterator it = values.begin();
        while (it != values.end())
        {
            const Boxed* value = *it;
            const TypedBoxed<T>* boxed = value->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(value) : NULL;
            hash += hasher(boxed->getValue());
            ++it;
        }
        return hash;
    }
    const std::type_info* mValueClass;
    std::wstring mName;
};

template <class T>
void AbstractProperty<T>::getUnboxedPossibleValues(boost::unordered_set<T>& values) const
{
    values.clear();
    const std::vector<Boxed*>& possibleValues = getPossibleValues();
    for (typename std::vector<Boxed*>::const_iterator it = possibleValues.begin(); it != possibleValues.end(); ++it)
    {
        const TypedBoxed<T>* boxed = static_cast<const TypedBoxed<T>*>(*it);
        if (!boxed->isA(typeid(T)))
            boxed = NULL;
        values.insert(boxed->getValue());
    }
}

#endif
