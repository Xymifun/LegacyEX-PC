#include "game/world/block/BlockStatePredicate.hpp"

#include <algorithm>
#include <cassert>

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

BlockStatePredicate::BlockStatePredicate(BlockStateDefinition* definition) : mDefinition(definition)
{
}

BlockStatePredicate* BlockStatePredicate::forBlock(Block* block)
{
    return new BlockStatePredicate(block->getBlockStateDefinition());
}

BlockStatePredicate* BlockStatePredicate::forDefinition(BlockStateDefinition* definition)
{
    return new BlockStatePredicate(definition);
}

bool BlockStatePredicate::apply(const BlockState* state) const
{
    if (state == NULL || state->getBlock() != mDefinition->getBlock())
    {
        return false;
    }
    if (!mPredicates.empty())
    {
        for (boost::unordered_map<Property*, const Predicate<Boxed*>*>::const_iterator it = mPredicates.begin();
             it != mPredicates.end(); ++it)
        {
            Boxed* value = state->getBoxedValue(it->first);
            if (!it->second->apply(value))
            {
                return false;
            }
        }
    }
    return true;
}

BlockStatePredicate* BlockStatePredicate::where(Property* property, const Predicate<Boxed*>* predicate)
{
    const std::vector<const Property*>& properties = mDefinition->getProperties();
    std::vector<const Property*>::const_iterator found = std::find(properties.begin(), properties.end(), property);
    assert(found != properties.end());
    mPredicates[property] = predicate;
    return this;
}
