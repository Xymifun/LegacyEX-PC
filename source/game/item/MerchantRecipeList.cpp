#include "game/item/MerchantRecipeList.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"

MerchantRecipeList::MerchantRecipeList() : std::vector<MerchantRecipe*>()
{
}

MerchantRecipeList::MerchantRecipeList(CompoundTag* tag) : std::vector<MerchantRecipe*>()
{
    load(tag);
}

MerchantRecipeList::~MerchantRecipeList()
{
    for (iterator it = std::vector<MerchantRecipe*>::begin(); it != std::vector<MerchantRecipe*>::end(); ++it)
    {
        delete *it;
    }
}

void MerchantRecipeList::load(CompoundTag* tag)
{
    ListTag* recipes = tag->getList(L"Recipes", Tag::TAG_COMPOUND);
    for (int i = 0; i < recipes->size(); ++i)
    {
        std::vector<MerchantRecipe*>::push_back(new MerchantRecipe(recipes->getCompound(i)));
    }
}

bool MerchantRecipeList::isRequiredItem(not_null_ptr<ItemInstance> supplied, not_null_ptr<ItemInstance> required)
{
    return ItemInstance::isSame(supplied, required);
}

MerchantRecipe* MerchantRecipeList::getRecipeFor(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second,
                                                 int hint)
{
    if (hint > 0 && hint < std::vector<MerchantRecipe*>::size())
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(hint);
        if (isRequiredItem(first, recipe->getBuyAItem())
            && ((second->isEmpty() && !recipe->hasSecondaryBuyItem())
                || (recipe->hasSecondaryBuyItem() && isRequiredItem(second, recipe->getBuyAItem()))))
        {
            if (first->getCount() >= recipe->getBuyAItem()->getCount()
                && (!recipe->hasSecondaryBuyItem() || second->getCount() >= recipe->getBuyBItem()->getCount()))
            {
                return recipe;
            }
        }
        return NULL;
    }

    for (unsigned int index = 0; index < std::vector<MerchantRecipe*>::size(); ++index)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(index);
        if (isRequiredItem(first, recipe->getBuyAItem()) && first->getCount() >= recipe->getBuyAItem()->getCount()
            && ((!recipe->hasSecondaryBuyItem() && second->isEmpty())
                || (recipe->hasSecondaryBuyItem() && isRequiredItem(second, recipe->getBuyBItem())
                    && second->getCount() >= recipe->getBuyBItem()->getCount())))
        {
            return recipe;
        }
    }
    return NULL;
}

bool MerchantRecipeList::addIfNewOrBetter(MerchantRecipe* recipe)
{
    bool result = false;
    for (int i = 0; i < std::vector<MerchantRecipe*>::size(); ++i)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(i);
        if (recipe->isSame(recipe))
        {
            if (recipe->isSameSameButBetter(recipe))
            {
                delete (*this)[i];
                (*this)[i] = recipe;
                result = true;
            }
            return result;
        }
    }
    std::vector<MerchantRecipe*>::push_back(recipe);
    return true;
}

MerchantRecipe* MerchantRecipeList::getMatchingRecipeFor(not_null_ptr<ItemInstance> first,
                                                         not_null_ptr<ItemInstance> second,
                                                         not_null_ptr<ItemInstance> result)
{
    for (unsigned int index = 0; index < std::vector<MerchantRecipe*>::size(); ++index)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(index);
        if (first->getItem() == recipe->getBuyAItem()->getItem()
            && first->getCount() >= recipe->getBuyAItem()->getCount()
            && result->getItem() == recipe->getSellItem()->getItem())
        {
            if (!recipe->hasSecondaryBuyItem()
                || (!second->isEmpty() && second->getItem() == recipe->getBuyBItem()->getItem()
                    && second->getCount() >= recipe->getBuyBItem()->getCount()))
            {
                return recipe;
            }
        }
    }
    return NULL;
}

void MerchantRecipeList::writeToStream(DataOutputStream* stream)
{
    stream->writeByte(static_cast<unsigned char>(std::vector<MerchantRecipe*>::size()));
    for (unsigned int i = 0; i < std::vector<MerchantRecipe*>::size(); ++i)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(i);
        stream->writeItem(recipe->getBuyAItem());
        stream->writeItem(recipe->getSellItem());
        not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
        stream->writeBoolean(!buyB->isEmpty());
        if (!buyB->isEmpty())
        {
            stream->writeItem(buyB);
        }
        stream->writeBoolean(recipe->isDeprecated());
        stream->writeInt(recipe->getUses());
        stream->writeInt(recipe->getMaxUses());
    }
}

void MerchantRecipeList::push_back(MerchantRecipe* recipe)
{
    std::vector<MerchantRecipe*>::push_back(recipe);
}

MerchantRecipeList* MerchantRecipeList::createFromStream(DataInputStream* stream)
{
    MerchantRecipeList* result = new MerchantRecipeList();
    int count = stream->readByte();
    for (int i = 0; i < count; ++i)
    {
        not_null_ptr<ItemInstance> buyA = stream->readItem();
        not_null_ptr<ItemInstance> sell = stream->readItem();
        not_null_ptr<ItemInstance> buyB = ItemInstance::EMPTY;
        if (stream->readBoolean())
        {
            buyB = stream->readItem();
        }
        bool deprecated = stream->readBoolean();
        int uses = stream->readInt();
        int maxUses = stream->readInt();
        MerchantRecipe* recipe = new MerchantRecipe(buyA, buyB, sell, uses, maxUses);
        if (deprecated)
        {
            recipe->enforceDeprecated();
        }
        result->push_back(recipe);
    }
    return result;
}

CompoundTag* MerchantRecipeList::createTag()
{
    CompoundTag* tag = new CompoundTag();
    ListTag* recipes = new ListTag();
    for (unsigned int i = 0; i < std::vector<MerchantRecipe*>::size(); ++i)
    {
        recipes->add(std::vector<MerchantRecipe*>::at(i)->createTag());
    }
    tag->put(L"Recipes", recipes);
    return tag;
}

MerchantRecipe* MerchantRecipeList::at(unsigned int index)
{
    return std::vector<MerchantRecipe*>::at(index);
}

MerchantRecipeList::iterator MerchantRecipeList::begin()
{
    return std::vector<MerchantRecipe*>::begin();
}

MerchantRecipeList::iterator MerchantRecipeList::end()
{
    return std::vector<MerchantRecipe*>::end();
}

int MerchantRecipeList::size()
{
    return static_cast<int>(std::vector<MerchantRecipe*>::size());
}

bool MerchantRecipeList::empty()
{
    return std::vector<MerchantRecipe*>::empty();
}

MerchantRecipeList::iterator MerchantRecipeList::erase(iterator position)
{
    delete *position;
    return std::vector<MerchantRecipe*>::erase(position);
}
