#ifndef GAME_ITEM_FOODITEM_HPP
#define GAME_ITEM_FOODITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class LivingEntity;
class MobEffectInstance;
class Player;

class FoodItem : public Item
{
public:
    FoodItem(int nutrition, float saturationModifier, bool isMeat);
    FoodItem(int nutrition, bool isMeat);
    virtual ~FoodItem() {}
    void _init();
    virtual not_null_ptr<ItemInstance> finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity);
    virtual void addEatEffect(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player);
    virtual int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const;
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    bool canEat(boost::shared_ptr<Player> player);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual int getNutrition(not_null_ptr<ItemInstance> itemInstance);
    virtual float getSaturationModifier(not_null_ptr<ItemInstance> itemInstance);
    virtual bool isMeat();
    virtual FoodItem* setEatEffect(MobEffectInstance* effect, float probability);
    virtual FoodItem* setCanAlwaysEat();
    void setIsFastFood();

protected:
    int mNutrition;
    float mSaturationModifier;
    bool mIsMeat;
    bool mCanAlwaysEat;
    bool mIsFastFood;
    MobEffectInstance* mEatEffect;
    float mEffectProbability;
};

#endif
