#ifndef GAME_WORLD_BLOCK_BLOCKSTATE_HPP
#define GAME_WORLD_BLOCK_BLOCKSTATE_HPP

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockBehaviours.hpp"
#include "game/world/block/BlockProperties.hpp"
#include "game/world/block/TypedBoxed.hpp"

class Block;
class Boxed;
class ItemInstance;
class Property;

template <typename T>
class AbstractProperty;

class BlockState : public BlockProperties, public BlockBehaviours
{
public:
    virtual const std::vector<const Property*>& getProperties() const = 0;
    virtual bool hasProperty(const Property* property) const = 0;
    virtual Boxed* getBoxedValue(const Property* property) const = 0;
    virtual const BlockState* setBoxedValue(const Property* property, Boxed* value) const = 0;
    virtual const BlockState* cycle(const Property* property) const = 0;
    virtual boost::unordered_map<const Property*, Boxed*>* getValues() const = 0;
    virtual Block* getBlock() const = 0;
    virtual std::wstring toString() const = 0;
    virtual bool equals(const BlockState* other) = 0;
    virtual int hashCode() const = 0;

    template <typename T>
    T getValue(const AbstractProperty<T>* property) const
    {
        return getTypedBoxed<T>(property)->getValue();
    }

    template <typename T>
    TypedBoxed<T>* getTypedBoxed(const AbstractProperty<T>* property) const
    {
        TypedBoxed<T>* boxed = static_cast<TypedBoxed<T>*>(getBoxedValue(property));
        return boxed->isA(typeid(T)) ? boxed : NULL;
    }

    template <typename T>
    const BlockState* setValue(const AbstractProperty<T>* property, T value) const
    {
        TypedBoxed<T> boxed(value);
        return setBoxedValue(property, &boxed);
    }

    int mElementId;
};

boost::shared_ptr<ItemInstance> createItem(const BlockState* state);

#endif
