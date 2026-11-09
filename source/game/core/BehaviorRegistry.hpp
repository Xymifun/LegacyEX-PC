#ifndef GAME_CORE_BEHAVIORREGISTRY_HPP
#define GAME_CORE_BEHAVIORREGISTRY_HPP

#include <boost/unordered_map.hpp>

#include "game/item/DispenseItemBehavior.hpp"
#include "game/item/Item.hpp"

class BehaviorRegistry : public boost::unordered_map<Item*, DispenseItemBehavior*>
{
public:
    BehaviorRegistry(DispenseItemBehavior* defaultBehavior);
    ~BehaviorRegistry();

    void add(Item* item, DispenseItemBehavior* behavior);
    DispenseItemBehavior* get(Item* item);

private:
    DispenseItemBehavior* mDefaultBehavior;
};

#endif
