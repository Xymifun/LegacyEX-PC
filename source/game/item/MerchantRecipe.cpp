#include "game/item/MerchantRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"

MerchantRecipe::MerchantRecipe(CompoundTag* tag)
    : mBuyA(ItemInstance::EMPTY),
      mBuyB(ItemInstance::EMPTY),
      mSell(ItemInstance::EMPTY),
      mUses(0)
{
    load(tag);
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> buyB,
                               not_null_ptr<ItemInstance> sell, int uses, int maxUses)
{
    _init(buyA, buyB, sell);
    mUses = uses;
    mMaxUses = maxUses;
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> sell)
{
    _init(buyA, ItemInstance::EMPTY, sell);
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, Item* sell)
{
    _init(buyA, ItemInstance::EMPTY, not_null_ptr<ItemInstance>(new ItemInstance(sell)));
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, Block* sell)
{
    _init(buyA, ItemInstance::EMPTY, not_null_ptr<ItemInstance>(new ItemInstance(sell)));
}

void MerchantRecipe::_init(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> buyB,
                           not_null_ptr<ItemInstance> sell)
{
    mBuyA = buyA;
    mBuyB = buyB;
    mSell = sell;
    mUses = 0;
    mMaxUses = 7;
    mRewardExp = true;
}

void MerchantRecipe::load(CompoundTag* tag)
{
    CompoundTag* buyTag = tag->getCompound(L"buy");
    mBuyA = not_null_ptr<ItemInstance>(new ItemInstance(buyTag));
    CompoundTag* sellTag = tag->getCompound(L"sell");
    mSell = not_null_ptr<ItemInstance>(new ItemInstance(sellTag));
    if (tag->contains(L"buyB", Tag::TAG_COMPOUND))
    {
        mBuyB = not_null_ptr<ItemInstance>(new ItemInstance(tag->getCompound(L"buyB")));
    }
    if (tag->contains(L"uses", Tag::TAG_ANY_NUMERIC))
    {
        mUses = tag->getInt(L"uses");
    }
    if (tag->contains(L"maxUses", Tag::TAG_ANY_NUMERIC))
    {
        mMaxUses = tag->getInt(L"maxUses");
    }
    else
    {
        mMaxUses = 7;
    }
    if (tag->contains(L"rewardExp", Tag::TAG_BYTE))
    {
        mRewardExp = tag->getBoolean(L"rewardExp");
    }
    else
    {
        mRewardExp = true;
    }
}

not_null_ptr<ItemInstance> MerchantRecipe::getBuyAItem()
{
    return mBuyA;
}
not_null_ptr<ItemInstance> MerchantRecipe::getBuyBItem()
{
    return mBuyB;
}
bool MerchantRecipe::hasSecondaryBuyItem()
{
    return !mBuyB->isEmpty();
}
not_null_ptr<ItemInstance> MerchantRecipe::getSellItem()
{
    return mSell;
}
bool MerchantRecipe::isSame(MerchantRecipe* other)
{
    if (mBuyA->getItem() != other->mBuyA->getItem() || mSell->getItem() != other->mSell->getItem())
    {
        return false;
    }
    return (mBuyB->isEmpty() && other->mBuyB->isEmpty())
           || (!mBuyB->isEmpty() && !other->mBuyB->isEmpty() && mBuyB->getItem() == other->mBuyB->getItem());
}
bool MerchantRecipe::isSameSameButBetter(MerchantRecipe* other)
{
    return isSame(other)
           && (mBuyA->getCount() < other->mBuyA->getCount()
               || (!mBuyB->isEmpty() && mBuyB->getCount() < other->mBuyB->getCount()));
}
int MerchantRecipe::getUses()
{
    return mUses;
}
int MerchantRecipe::getMaxUses()
{
    return mMaxUses;
}
void MerchantRecipe::increaseUses()
{
    ++mUses;
}
void MerchantRecipe::increaseMaxUses(int amount)
{
    mMaxUses += amount;
}
bool MerchantRecipe::isDeprecated()
{
    bool hasSellItem = !mSell->isEmpty();
    bool deprecated = mUses >= mMaxUses || !hasSellItem;
    return deprecated;
}
void MerchantRecipe::enforceDeprecated()
{
    mUses = mMaxUses;
}
bool MerchantRecipe::shouldRewardExp()
{
    return mRewardExp;
}
CompoundTag* MerchantRecipe::createTag()
{
    CompoundTag* tag = new CompoundTag();
    tag->put(L"buy", mBuyA->save(new CompoundTag()));
    tag->put(L"sell", mSell->save(new CompoundTag()));
    if (!mBuyB->isEmpty())
    {
        tag->put(L"buyB", mBuyB->save(new CompoundTag()));
    }
    tag->putInt(L"uses", mUses);
    tag->putInt(L"maxUses", mMaxUses);
    tag->putBoolean(L"rewardExp", mRewardExp);
    return tag;
}
