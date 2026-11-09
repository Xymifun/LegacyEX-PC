#ifndef GAME_UI_ITEMTOOLTIPDATAHOLDER_HPP
#define GAME_UI_ITEMTOOLTIPDATAHOLDER_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionResult.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class MultiPlayerLevel;
class MultiplayerLocalPlayer;

class ItemTooltipDataHolder
{
public:
    boost::shared_ptr<MultiplayerLocalPlayer> m_pPlayer;
    not_null_ptr<ItemInstance> m_pItemInstance;
    MultiPlayerLevel* m_pLevel;
    InteractionResult::EInteractionResult m_eInteractionResult;

    ItemTooltipDataHolder(boost::shared_ptr<MultiplayerLocalPlayer> pPlayer, not_null_ptr<ItemInstance> pItemInstance,
                          MultiPlayerLevel* pLevel, InteractionResult::EInteractionResult eInteractionResult);
};

#endif
