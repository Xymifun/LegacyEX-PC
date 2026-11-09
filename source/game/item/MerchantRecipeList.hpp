#ifndef GAME_ITEM_MERCHANTRECIPELIST_HPP
#define GAME_ITEM_MERCHANTRECIPELIST_HPP

#include <vector>

#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DataInputStream;
class DataOutputStream;
class ItemInstance;
class MerchantRecipe;

class MerchantRecipeList : public std::vector<MerchantRecipe*>
{
public:
    MerchantRecipeList();
    explicit MerchantRecipeList(CompoundTag* tag);
    ~MerchantRecipeList();

    void load(CompoundTag* tag);
    bool isRequiredItem(not_null_ptr<ItemInstance> supplied, not_null_ptr<ItemInstance> required);
    MerchantRecipe* getRecipeFor(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second, int hint);
    bool addIfNewOrBetter(MerchantRecipe* recipe);
    MerchantRecipe* getMatchingRecipeFor(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second,
                                         not_null_ptr<ItemInstance> result);
    void writeToStream(DataOutputStream* stream);
    void push_back(MerchantRecipe* recipe);
    static MerchantRecipeList* createFromStream(DataInputStream* stream);
    CompoundTag* createTag();
    MerchantRecipe* at(unsigned int index);
    iterator begin();
    iterator end();
    int size();
    bool empty();
    iterator erase(iterator position);
};

#endif
