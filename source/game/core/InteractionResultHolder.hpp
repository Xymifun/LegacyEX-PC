#ifndef GAME_CORE_INTERACTIONRESULTHOLDER_HPP
#define GAME_CORE_INTERACTIONRESULTHOLDER_HPP

#include "game/core/InteractionResult.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;

class InteractionResultHolder
{
public:
    InteractionResultHolder(InteractionResult::EInteractionResult result, const not_null_ptr<ItemInstance>& object)
        : result(result),
          object(object)
    {
    }

    InteractionResult::EInteractionResult getResult() const { return result; }
    const not_null_ptr<ItemInstance>& getObject() const { return object; }

private:
    InteractionResult::EInteractionResult result;
    not_null_ptr<ItemInstance> object;
};

#endif
