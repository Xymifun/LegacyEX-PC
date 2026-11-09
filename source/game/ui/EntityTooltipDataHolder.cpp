#include "game/ui/EntityTooltipDataHolder.hpp"

EntityTooltipDataHolder::EntityTooltipDataHolder(boost::shared_ptr<MultiplayerLocalPlayer> pPlayer,
                                                 not_null_ptr<ItemInstance> pItemInstance, bool bAllowedToHurt,
                                                 bool bAllowedToInteract)
{
    m_pPlayer = pPlayer;
    m_pItemInstance = pItemInstance;
    m_bAllowedToHurt = bAllowedToHurt;
    m_bAllowedToInteract = bAllowedToInteract;
}
