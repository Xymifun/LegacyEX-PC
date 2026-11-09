#include "game/item/CarrotOnAStickItem.hpp"
#include "game/core/StringIds.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/EntityType.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/Pig.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"

CarrotOnAStickItem::CarrotOnAStickItem() : Item()
{
    setMaxStackSize(1);
    setMaxDamage(0x19);
}

bool CarrotOnAStickItem::isHandEquipped()
{
    return true;
}

bool CarrotOnAStickItem::isMirroredArt()
{
    return true;
}

bool CarrotOnAStickItem::TestUse(Level*, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (player->isPassenger() && eTYPE_DERIVED_FROM(EntityType::PIG, player->getVehicle()->GetType()))
    {
        boost::shared_ptr<Pig> pig = boost::static_pointer_cast<Pig>(player->getVehicle());
        if (itemInstance->getMaxDamage() - itemInstance->getAuxValue() > 6 && pig->testboost())
        {
            return true;
        }
    }

    return false;
}

InteractionResultHolder CarrotOnAStickItem::use(Level* level, boost::shared_ptr<Player> player,
                                                InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (level->mIsClientSide)
    {
        return InteractionResultHolder(TestUse(level, player, hand) ? InteractionResult::eInteractionResult_Success
                                                                    : InteractionResult::eInteractionResult_Pass,
                                       itemInstance);
    }

    if (player->isPassenger() && eTYPE_DERIVED_FROM(EntityType::PIG, player->getVehicle()->GetType()))
    {
        boost::shared_ptr<Pig> pig = boost::static_pointer_cast<Pig>(player->getVehicle());
        if (itemInstance->getMaxDamage() - itemInstance->getAuxValue() > 6 && pig->boost())
        {
            itemInstance->hurtAndBreak(7, player);

            if (!itemInstance->isEmpty())
            {
                return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
            }

            not_null_ptr<ItemInstance> replacement(new ItemInstance(Items::fishingRod));
            replacement->setTag(itemInstance->getTag());
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success, replacement);
        }
    }

    Player* awardee = player.operator->();
    Stat* stat = gStats->get_itemsUsed(VALIDgetId(this));
    awardee->awardStat(stat, GenericStats::param_itemsUsed(player, itemInstance));
    return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
}

unsigned int CarrotOnAStickItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success)
    {
        return IDS_TOOLTIPS_BOOST;
    }
    return IDS_NONE;
}
