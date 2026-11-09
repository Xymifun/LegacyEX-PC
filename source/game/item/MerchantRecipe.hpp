#ifndef GAME_ITEM_MERCHANTRECIPE_HPP
#define GAME_ITEM_MERCHANTRECIPE_HPP

#include "game/item/not_null_ptr.hpp"

class Block;
class CompoundTag;
class Item;
class ItemInstance;

class MerchantRecipe
{
public:
    MerchantRecipe(CompoundTag* tag);
    MerchantRecipe(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> buyB, not_null_ptr<ItemInstance> sell,
                   int uses, int maxUses);
    MerchantRecipe(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> sell);
    MerchantRecipe(not_null_ptr<ItemInstance> buyA, Item* sell);
    MerchantRecipe(not_null_ptr<ItemInstance> buyA, Block* sell);

    void _init(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> buyB, not_null_ptr<ItemInstance> sell);
    void load(CompoundTag* tag);
    not_null_ptr<ItemInstance> getBuyAItem();
    not_null_ptr<ItemInstance> getBuyBItem();
    bool hasSecondaryBuyItem();
    not_null_ptr<ItemInstance> getSellItem();
    bool isSame(MerchantRecipe* other);
    bool isSameSameButBetter(MerchantRecipe* other);
    int getUses();
    int getMaxUses();
    void increaseUses();
    void increaseMaxUses(int amount);
    bool isDeprecated();
    void enforceDeprecated();
    bool shouldRewardExp();
    CompoundTag* createTag();

private:
    not_null_ptr<ItemInstance> mBuyA;
    not_null_ptr<ItemInstance> mBuyB;
    not_null_ptr<ItemInstance> mSell;
    int mUses;
    int mMaxUses;
    bool mRewardExp;
};

#endif
