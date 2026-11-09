#ifndef GAME_ITEM_POTIONITEM_HPP
#define GAME_ITEM_POTIONITEM_HPP

#include <stdint.h>

#include "game/item/Item.hpp"
#include "game/core/InteractionResultHolder.hpp"

class IconRegister;
class ItemTooltipDataHolder;
class TextureAtlasSprite;

class PotionItem : public Item
{
public:
    PotionItem();
    virtual ~PotionItem() {}

    virtual not_null_ptr<ItemInstance> getDefaultInstance();
    virtual not_null_ptr<ItemInstance> finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual bool hasMultipleSpriteLayers();
    virtual void registerIcons(IconRegister* iconRegister);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);

    virtual uint32_t getPotionType() const;
    virtual std::wstring getBottleTextureName();
    virtual int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const;

private:
    TextureAtlasSprite* mIcon;
    TextureAtlasSprite* mLayerIcon;
};

#endif
