#include "game/item/FireworksItem.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/L10N.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/item/FireworkChargeItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

const std::wstring FireworksItem::TAG_FIREWORKS(L"Fireworks");
const std::wstring FireworksItem::TAG_EXPLOSION(L"Explosion");
const std::wstring FireworksItem::TAG_EXPLOSIONS(L"Explosions");
const std::wstring FireworksItem::TAG_FLIGHT(L"Flight");
const std::wstring FireworksItem::TAG_E_TYPE(L"Type");
const std::wstring FireworksItem::TAG_E_TRAIL(L"Trail");
const std::wstring FireworksItem::TAG_E_FLICKER(L"Flicker");
const std::wstring FireworksItem::TAG_E_COLORS(L"Colors");
const std::wstring FireworksItem::TAG_E_FADECOLORS(L"FadeColors");

FireworksItem::FireworksItem() : Item()
{
}

InteractionResult::EInteractionResult FireworksItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                           const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                           const Direction* direction, float hitX, float hitY,
                                                           float hitZ, bool simulate)
{
    if (!level->mIsClientSide)
    {
        not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
        {
            boost::shared_ptr<FireworksRocketEntity> rocket(new FireworksRocketEntity(level, player, itemInstance));
            level->addEntity(rocket);
            player->getCooldowns()->addCooldown(this, 10);
        }
        else
        {
            boost::shared_ptr<FireworksRocketEntity> rocket(
                new FireworksRocketEntity(level, static_cast<float>(pos.x) + hitX, static_cast<float>(pos.y) + hitY,
                                          static_cast<float>(pos.z) + hitZ, itemInstance));
            level->addEntity(rocket);

            if (!player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
        }
    }

    return InteractionResult::eInteractionResult_Success;
}

InteractionResultHolder FireworksItem::use(Level* level, boost::shared_ptr<Player> player,
                                           InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        if (!level->mIsClientSide)
        {
            boost::shared_ptr<FireworksRocketEntity> rocket(new FireworksRocketEntity(level, player, itemInstance));
            level->addEntity(rocket);
            player->getCooldowns()->addCooldown(this, 10);
        }
        else if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureOverrideHandSwing())
        {
            player->swing(InteractionHand::eInteractionHand_MainHand);
        }

        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
    }
    else if (player->isFallFlying())
    {
        if (!level->mIsClientSide)
        {
            boost::shared_ptr<FireworksRocketEntity> rocket(new FireworksRocketEntity(level, itemInstance, player));
            level->addEntity(rocket);
            if (!player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
        }
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
    }
    else
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }
}

void FireworksItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                    std::vector<HtmlString>* lines, bool advanced)
{
    if (!itemInstance->hasTag())
    {
        return;
    }

    CompoundTag* fireworks = itemInstance->getTag()->getCompound(TAG_FIREWORKS);
    if (fireworks == NULL)
    {
        return;
    }

    if (fireworks->contains(TAG_FLIGHT, 99))
    {
        lines->push_back(HtmlString(std::wstring(L10N::GetString(IDS_ITEM_FIREWORKS_FLIGHT)) + L" "
                                        + _toString(static_cast<int>(fireworks->getByte(TAG_FLIGHT))),
                                    eMinecraftColour_NOTSET, false, false, 0));
    }

    ListTag* explosions = fireworks->getList(TAG_EXPLOSIONS, 10);
    if (explosions->isEmpty())
    {
        return;
    }

    for (int i = 0; i < explosions->size(); ++i)
    {
        CompoundTag* explosion = explosions->getCompound(i);
        std::vector<HtmlString> chargeLines;
        FireworkChargeItem::appendHoverText(explosion, &chargeLines);
        if (!chargeLines.empty())
        {
            for (int j = 1; j < chargeLines.size(); ++j)
            {
                chargeLines[j].mIndent = true;
            }
            lines->insert(lines->end(), chargeLines.begin(), chargeLines.end());
        }
    }
}

unsigned int FireworksItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return tooltipData.m_pPlayer->isFallFlying() ? 0x568A07BEu : IDS_NONE;
}
