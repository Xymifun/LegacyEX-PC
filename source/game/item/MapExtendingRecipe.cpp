#include "game/item/MapExtendingRecipe.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/core/MapDecoration.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/world/Level.hpp"

static ItemInstance** gMapExtendingIngredients = NULL;

MapExtendingRecipe::~MapExtendingRecipe()
{
}

void MapExtendingRecipe::StaticCtor()
{
    gMapExtendingIngredients = new ItemInstance*[9];
    gMapExtendingIngredients[0] = new ItemInstance(Items::paper);
    gMapExtendingIngredients[1] = new ItemInstance(Items::paper);
    gMapExtendingIngredients[2] = new ItemInstance(Items::paper);
    gMapExtendingIngredients[3] = new ItemInstance(Items::paper);
    gMapExtendingIngredients[4] = new ItemInstance(Items::map, 0, -1);
    gMapExtendingIngredients[5] = new ItemInstance(Items::paper);
    gMapExtendingIngredients[6] = new ItemInstance(Items::paper);
    gMapExtendingIngredients[7] = new ItemInstance(Items::paper);
    gMapExtendingIngredients[8] = new ItemInstance(Items::paper);
}

bool MapExtendingRecipe::isExplorationMap(const boost::shared_ptr<MapItemSavedData>& data)
{
    if (!data->mDecorations.empty())
    {
        boost::unordered_map<int, MapDecoration>::iterator it;
        for (it = data->mDecorations.begin(); it != data->mDecorations.end(); ++it)
        {
            const MapDecoration::Type* type = it->second.getType();
            if (type == &MapDecoration::Type::TYPES[14] || type == &MapDecoration::Type::TYPES[15]
                || type == &MapDecoration::Type::TYPES[20])
            {
                return true;
            }
        }
    }
    return false;
}

bool MapExtendingRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    if (!ShapedRecipe::matches(craftingContainer, level))
    {
        return false;
    }

    not_null_ptr<ItemInstance> map = ItemInstance::EMPTY;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize() && map->isEmpty(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->getItem() == Items::filledMap)
        {
            map = item;
        }
    }

    if (map->isEmpty())
    {
        return false;
    }

    boost::shared_ptr<MapItemSavedData> data = static_cast<MapItem*>(Items::filledMap)->getSavedData(map, level);
    if (!data)
    {
        return false;
    }
    if (isExplorationMap(data))
    {
        return false;
    }
    return data->mScale < 4;
}

not_null_ptr<ItemInstance> MapExtendingRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    not_null_ptr<ItemInstance> map = ItemInstance::EMPTY;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize() && map->isEmpty(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->getItem() == Items::filledMap)
        {
            map = item;
        }
    }

    map = map->copy();
    map->setCount(1);
    if (map->getTag() == NULL)
    {
        map->setTag(new CompoundTag());
    }
    map->getTag()->putInt(L"map_scale_direction", 1);
    return map;
}

Recipe::_eGroupType MapExtendingRecipe::getGroup()
{
    return eGroupType_None;
}

bool MapExtendingRecipe::recipeRequires(int itemId)
{
    return false;
}

void MapExtendingRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}
