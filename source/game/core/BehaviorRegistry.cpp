#include "game/core/BehaviorRegistry.hpp"

BehaviorRegistry::BehaviorRegistry(DispenseItemBehavior* defaultBehavior)
    : boost::unordered_map<Item*, DispenseItemBehavior*>(11),
      mDefaultBehavior(defaultBehavior)
{
}

BehaviorRegistry::~BehaviorRegistry()
{
    for (iterator it = begin(); it != end(); ++it)
    {
        delete it->second;
    }

    delete mDefaultBehavior;
}

void BehaviorRegistry::add(Item* item, DispenseItemBehavior* behavior)
{
    insert(value_type(item, behavior));
}

DispenseItemBehavior* BehaviorRegistry::get(Item* item)
{
    iterator it = find(item);
    if (it == end())
    {
        return mDefaultBehavior;
    }

    return it->second;
}
