#ifndef GAME_WORLD_BLOCK_ENUMPROPERTY_HPP
#define GAME_WORLD_BLOCK_ENUMPROPERTY_HPP

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Property.hpp"

template <class T>
class EnumProperty : public AbstractProperty<T>
{
public:
    static EnumProperty<T>* create(const std::wstring& name, const std::type_info& valueClass,
                                   const boost::unordered_set<T>& values, const Predicate<T>* filter)
    {
        boost::unordered_set<T> allowedValues;

        for (typename boost::unordered_set<T>::const_iterator it = values.begin(); it != values.end(); ++it)
        {
            if (filter->apply(*it))
            {
                allowedValues.insert(*it);
            }
        }

        return new EnumProperty<T>(name, valueClass, allowedValues);
    }

    static EnumProperty<T>* create(const std::wstring& name, const std::type_info& valueClass,
                                   const std::vector<T>& values, const Predicate<T>* filter)
    {
        boost::unordered_set<T> allowedValues;

        for (typename std::vector<T>::const_iterator it = values.begin(); it != values.end(); ++it)
        {
            if (filter->apply(*it))
            {
                allowedValues.insert(*it);
            }
        }

        return new EnumProperty<T>(name, valueClass, allowedValues);
    }

    virtual ~EnumProperty()
    {
        for (std::vector<Boxed*>::iterator it = mPossibleValues.begin(); it != mPossibleValues.end(); ++it)
        {
            delete *it;
        }
    }

    virtual const std::vector<Boxed*>& getPossibleValues() const { return mPossibleValues; }
    virtual bool equals(const Property* other) const
    {
        if (this == other)
        {
            return true;
        }

        const EnumProperty<T>* property = dynamic_cast<const EnumProperty<T>*>(other);
        if (property != NULL && AbstractProperty<T>::equals(other))
        {
            return mPossibleValues == property->mPossibleValues && mValuesByName == property->mValuesByName;
        }

        return false;
    }
    virtual int hashCode() const
    {
        return ((int)((std::size_t)this->mValueClass * 31U) + javaStringHashCode(this->mName)) * 31
               + AbstractProperty<T>::hashBoxedSet(mPossibleValues);
    }
    virtual unsigned int getValueCount() const { return mPossibleValues.size(); }
    virtual Boxed* getValueAtIndex(unsigned int index) const { return mPossibleValues.at(index); }
    virtual unsigned int getIndexForValue(Boxed* value) const
    {
        unsigned int index = 0;

        for (std::vector<Boxed*>::const_iterator it = mPossibleValues.begin(); it != mPossibleValues.end(); ++it)
        {
            if (*value == *it)
            {
                return index;
            }

            ++index;
        }

        return 0;
    }
    virtual std::wstring getName(const T& value) const { return value->getSerializedName(); }
    virtual T getUnboxedValue(const std::wstring& name) const
    {
        typename ValuesByName::const_iterator entry = mValuesByName.find(name);
        if (entry != mValuesByName.end())
        {
            return entry->second;
        }

        return NULL;
    }

protected:
    typedef boost::unordered_map<std::wstring, T> ValuesByName;

    EnumProperty(const std::wstring& name, const std::type_info& valueClass, const boost::unordered_set<T>& values)
        : AbstractProperty<T>(name, valueClass)
    {
        for (typename boost::unordered_set<T>::const_iterator it = values.begin(); it != values.end(); ++it)
        {
            std::wstring valueName = getName(*it);

            mValuesByName.find(valueName);
            mValuesByName[valueName] = *it;

            mPossibleValues.push_back(new TypedBoxed<T>(*it));
        }

        this->updateCachedHashCode();
    }

    std::vector<Boxed*> mPossibleValues;
    ValuesByName mValuesByName;
};

#endif
