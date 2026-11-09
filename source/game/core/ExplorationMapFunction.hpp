#ifndef GAME_CORE_EXPLORATIONMAPFUNCTION_HPP
#define GAME_CORE_EXPLORATIONMAPFUNCTION_HPP

#include <string>

#include "game/core/MapDecoration.hpp"
#include "game/item/LootItemFunction.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class LootTableContext;
class Random;

class ExplorationMapFunction : public LootItemFunction
{
public:
    ExplorationMapFunction(std::vector<LootItemCondition*>* conditions, const std::wstring& destination);
    virtual ~ExplorationMapFunction() {}

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);
    const MapDecoration::Type* getType(const std::wstring& destination);
    unsigned int getTranslatableName(const std::wstring& destination);

private:
    std::wstring mDestination;
};

#endif
