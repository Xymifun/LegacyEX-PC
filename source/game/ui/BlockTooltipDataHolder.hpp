#ifndef GAME_UI_BLOCKTOOLTIPDATAHOLDER_HPP
#define GAME_UI_BLOCKTOOLTIPDATAHOLDER_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionResult.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"

class BlockState;
class ItemInstance;
class MultiPlayerLevel;
class MultiplayerLocalPlayer;

class BlockTooltipDataHolder
{
public:
    boost::shared_ptr<MultiplayerLocalPlayer> m_pPlayer;
    MultiPlayerLevel* m_pLevel;
    not_null_ptr<ItemInstance> m_pItemInstance;
    const BlockState* m_pBlockState;
    BlockPos m_blockPos;
    InteractionResult::EInteractionResult m_eInteractionResult;
    bool m_bUseBlock;
    bool m_bCanDestroy;
    bool m_bUseItemOnBlock;
    bool m_bCanPlace;

    BlockTooltipDataHolder(boost::shared_ptr<MultiplayerLocalPlayer> pPlayer, MultiPlayerLevel* pLevel,
                           not_null_ptr<ItemInstance> pItemInstance, const BlockState* pBlockState, BlockPos blockPos,
                           InteractionResult::EInteractionResult eInteractionResult, bool bUseBlock,
                           bool bUseItemOnBlock, bool bCanDestroy, bool bCanPlace);
};

#endif
