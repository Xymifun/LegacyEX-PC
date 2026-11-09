#include "game/world/block/PotatoBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include "game/core/StringConversion.hpp"

#include <string>

static const AABB* SHAPE_BY_AGE[8] = {
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.1875, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.25, 1.0),  AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.3125, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.4375, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0),   AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5625, 1.0)};

PotatoBlock::PotatoBlock() : CropBlock(false)
{
    ClearDerivedInit();
    CropBlock::DerivedInit();
}

Item* PotatoBlock::getBaseSeedId()
{
    return Items::potato;
}

Item* PotatoBlock::getBasePlantId()
{
    return Items::potato;
}

void PotatoBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                 int lootBonus)
{
    CropBlock::spawnResources(level, pos, state, chance, lootBonus);
    if (level->mIsClientSide || !isMaxAge(state) || level->mRandom->nextInt(50) != 0)
    {
        return;
    }
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::poisonousPotato)));
}

const AABB* PotatoBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[state->getValue(getAgeProperty())];
}

TextureAtlasSprite* PotatoBlock::getTexture(const Direction*, const BlockState* state)
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

void PotatoBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 4; ++i)
    {
        mStageIcons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + _toString<int>(i));
    }
}
