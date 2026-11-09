#include "game/core/ExplorationMapFunction.hpp"
#include "game/core/StringIds.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/LootTableContext.hpp"
#include "game/core/MapDecoration.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/generation/PlayerUID.hpp"

ExplorationMapFunction::ExplorationMapFunction(std::vector<LootItemCondition*>* conditions,
                                               const std::wstring& destination)
    : LootItemFunction(conditions),
      mDestination(destination)
{
}

const MapDecoration::Type* ExplorationMapFunction::getType(const std::wstring& destination)
{
    if (destination == LargeFeature::MONUMENT)
    {
        return &MapDecoration::Type::TYPES[15];
    }
    if (destination == LargeFeature::MANSION)
    {
        return &MapDecoration::Type::TYPES[14];
    }
    if (destination == LargeFeature::BURIED_TREASURE)
    {
        return &MapDecoration::Type::TYPES[20];
    }
    return &MapDecoration::Type::TYPES[20];
}

unsigned int ExplorationMapFunction::getTranslatableName(const std::wstring& destination)
{
    if (destination == LargeFeature::MONUMENT)
    {
        return IDS_ITEM_FILLED_MAP_OCEAN_MONUMENT;
    }
    if (destination == LargeFeature::MANSION)
    {
        return IDS_ITEM_FILLED_MAP_WOODLAND_MANSION;
    }
    if (destination == LargeFeature::BURIED_TREASURE)
    {
        return 0xd2f653b5;
    }
    return IDS_DESC_MAP;
}

not_null_ptr<ItemInstance> ExplorationMapFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random*,
                                                         LootTableContext& context)
{
    boost::shared_ptr<Entity> entity = context.getEntity(LootTableContext::THIS);
    Level* level = entity ? entity->mLevel : context.mLevel;
    Vec3* position = Vec3::newTemp(0.0, 0.0, 0.0);
    if (entity)
    {
        position = entity->getPos(0.0f);
    }
    BlockPos* featurePosition = level->findNearestMapFeature(mDestination, BlockPos(position), false);
    if (featurePosition == NULL)
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::nautilusCore, 1));
    }

    const MapDecoration::Type* type = getType(mDestination);
    int scale = type == &MapDecoration::Type::TYPES[20] ? 1 : 2;
    not_null_ptr<ItemInstance> map = MapItem::setupNewMap(PlayerUID(0), level, 0, (double)featurePosition->x,
                                                          (double)featurePosition->z, (unsigned char)scale, true, true);
    MapItem::renderBiomePreviewMap(level, map);
    MapItemSavedData::addTargetDecoration(map, *featurePosition, L"+", type);
    map->setTranslatableName(getTranslatableName(mDestination));
    delete featurePosition;
    return map;
}
