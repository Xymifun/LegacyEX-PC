#include "game/world/block/CarrotBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include "game/core/StringConversion.hpp"

#include <string>

static const AABB* SHAPE_BY_AGE[8] = {
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.1875, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.25, 1.0),  AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.3125, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.4375, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0),   AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5625, 1.0)};

const AABB* CarrotBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return SHAPE_BY_AGE[state->getValue(getAgeProperty())];
}

void CarrotBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 4; ++i)
    {
        mStageIcons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + _toString<int>(i));
    }
}

TextureAtlasSprite* CarrotBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int age = state->getValue(AGE);
    if (age < 7)
    {
        if (age == 6)
        {
            age = 5;
        }
        return mStageIcons[age >> 1];
    }
    return mStageIcons[3];
}

Item* CarrotBlock::getBaseSeedId()
{
    return Items::carrot;
}

Item* CarrotBlock::getBasePlantId()
{
    return Items::carrot;
}
