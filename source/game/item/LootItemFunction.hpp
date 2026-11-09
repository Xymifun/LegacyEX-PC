#ifndef GAME_ITEM_LOOTITEMFUNCTION_HPP
#define GAME_ITEM_LOOTITEMFUNCTION_HPP

#include <vector>

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class LootItemFunction
{
public:
    LootItemFunction(std::vector<LootItemCondition*>* conditions);

    const std::vector<LootItemCondition*>* getConditions() const { return conditions; }

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context) = 0;

protected:
    std::vector<LootItemCondition*>* conditions;
};

#endif
