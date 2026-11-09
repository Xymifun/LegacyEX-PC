#ifndef GAME_ITEM_BOWITEM_HPP
#define GAME_ITEM_BOWITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class IconRegister;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class LivingEntity;
class Player;
class TextureAtlasSprite;

class BowItem : public Item
{
public:
    BowItem();

    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual int getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const;
    virtual int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    virtual void releaseUsing(not_null_ptr<ItemInstance> itemInstance, Level* level,
                              boost::shared_ptr<LivingEntity> entity, int timeLeft);
    virtual int getEnchantmentValue();
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int GetOverrideCount();
    virtual int GetOverrideCountColour();
    virtual TextureAtlasSprite* GetOverrideCountIcon(not_null_ptr<ItemInstance> itemInstance);

    bool isValidProjectile(not_null_ptr<ItemInstance> itemInstance);
    not_null_ptr<ItemInstance> findProjectile(boost::shared_ptr<Player> player);

    InteractionResultHolder fjUseWithProjectile(Level* level, boost::shared_ptr<Player> player,
                                                InteractionHand::EInteractionHand hand,
                                                not_null_ptr<ItemInstance> projectile);
    void fjReleaseUsingWithProjectile(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                      boost::shared_ptr<LivingEntity> entity, int timeLeft,
                                      not_null_ptr<ItemInstance> projectile);

    TextureAtlasSprite* getDrawnIcon(int stage);

    static int GetMaxBowDuration();
    static float getPowerForTime(int charge);

private:
    TextureAtlasSprite* mDrawnIcons[3];
};

#endif
