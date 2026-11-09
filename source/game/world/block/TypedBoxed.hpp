#ifndef GAME_WORLD_BLOCK_TYPEDBOXED_HPP
#define GAME_WORLD_BLOCK_TYPEDBOXED_HPP

#include <string>
#include <typeinfo>

#include <boost/functional/hash.hpp>

#include "game/core/Boxed.hpp"
#include "game/core/StringRepresentable.hpp"

template <class T> std::wstring _toString(T value);

template <class T>
inline std::wstring _toString(T* value)
{
    const StringRepresentable* representable = value;
    if (representable == NULL)
    {
        return L"NULL";
    }

    return representable->toString();
}

template <class T>
class TypedBoxed : public Boxed
{
public:
    TypedBoxed(const T& value) : Boxed(&typeid(T)), mValue(value) {}
    virtual ~TypedBoxed() {}

    const T& getValue() const { return this->mValue; }

    virtual unsigned int hashOf() const
    {
        T value = this->mValue;
        return boost::hash_value(value);
    }
    virtual bool operator==(const Boxed* other) const
    {
        return other->isA(this->mValueClass)
               && this->mValue == (other->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(other) : NULL)->mValue;
    }
    virtual void setValue(const Boxed* other)
    {
        const TypedBoxed<T>* boxed = other->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(other) : NULL;
        this->mValue = boxed->mValue;
    }
    virtual std::wstring toString() const { return _toString(this->mValue); }

private:
    T mValue;
};

#endif
