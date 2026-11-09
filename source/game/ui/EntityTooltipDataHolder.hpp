#ifndef GAME_UI_ENTITYTOOLTIPDATAHOLDER_HPP
#define GAME_UI_ENTITYTOOLTIPDATAHOLDER_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class MultiplayerLocalPlayer;

class EntityTooltipDataHolder
{
public:
    boost::shared_ptr<MultiplayerLocalPlayer> m_pPlayer;
    not_null_ptr<ItemInstance> m_pItemInstance;
    bool m_bAllowedToHurt;
    bool m_bAllowedToInteract;

    EntityTooltipDataHolder(boost::shared_ptr<MultiplayerLocalPlayer> pPlayer, not_null_ptr<ItemInstance> pItemInstance,
                            bool bAllowedToHurt, bool bAllowedToInteract);
};

#endif
