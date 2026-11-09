#include "game/entity/projectile/TippedArrowItem.hpp"

#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/minigame/EMiniGameId.hpp"

not_null_ptr<ItemInstance> TippedArrowItem::getDefaultInstance()
{
    return PotionUtils::setPotion(Item::getDefaultInstance(), Potions::POISON);
}

boost::shared_ptr<Arrow> TippedArrowItem::createArrow(Level* level, not_null_ptr<ItemInstance> itemInstance,
                                                      boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<TippableArrow> arrow(new TippableArrow(level, entity));
    arrow->setEffectsFromItem(itemInstance);
    return arrow;
}

void TippedArrowItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                      std::vector<HtmlString>* lines, bool)
{
    if (Minecraft::InMiniGame(eMiniGameId_Battle, false))
    {
        PotionUtils::addPotionTooltip(itemInstance, lines, 0.33333334f, player);
    }
    else
    {
        PotionUtils::addPotionTooltip(itemInstance, lines, 0.125f, player);
    }
}

std::wstring TippedArrowItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    return PotionUtils::getPotion(itemInstance)->getName(Potion::ARROW);
}

int TippedArrowItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    Potion* potion = PotionUtils::getPotion(itemInstance);
    return potion->getUseDescriptionId(Potion::ARROW);
}

bool TippedArrowItem::hasMultipleSpriteLayers()
{
    return true;
}

TextureAtlasSprite* TippedArrowItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (layer == 0)
    {
        return mArrowHeadIcon;
    }

    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

void TippedArrowItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mArrowHeadIcon = iconRegister->registerIcon(L"tipped_arrow_head");
}
