#ifndef GAME_WORLD_BLOCK_BLOCKSTATEPREDICATE_HPP
#define GAME_WORLD_BLOCK_BLOCKSTATEPREDICATE_HPP

#include <typeinfo>
#include <boost/unordered_map.hpp>

#include "game/core/Boxed.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Predicate.hpp"

class Block;
class BlockState;
class BlockStateDefinition;
class Property;

class BlockStatePredicate : public Predicate<const BlockState*>
{
public:
    template <typename T>
    class PredicateConversion : public Predicate<Boxed*>
    {
    public:
        explicit PredicateConversion(const Predicate<T>* predicate) : mPredicate(predicate) {}
        virtual bool apply(Boxed* value) const;
        virtual ~PredicateConversion();

    private:
        const Predicate<T>* mPredicate;
    };

    explicit BlockStatePredicate(BlockStateDefinition* definition);
    static BlockStatePredicate* forBlock(Block* block);
    static BlockStatePredicate* forDefinition(BlockStateDefinition* definition);
    virtual bool apply(const BlockState* state) const;
    BlockStatePredicate* where(Property* property, const Predicate<Boxed*>* predicate);

private:
    BlockStateDefinition* mDefinition;
    boost::unordered_map<Property*, const Predicate<Boxed*>*> mPredicates;
};

template <typename T>
bool BlockStatePredicate::PredicateConversion<T>::apply(Boxed* value) const
{
    return mPredicate->apply((value->isA(typeid(T)) ? static_cast<TypedBoxed<T>*>(value) : NULL)->getValue());
}

template <typename T>
BlockStatePredicate::PredicateConversion<T>::~PredicateConversion()
{
}

#endif
