#include "game/world/block/BeetrootBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include "game/core/StringConversion.hpp"

#include <cstring>
#include <string>

AbstractProperty<int>* BeetrootBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 3));

static const AABB* SHAPE_BY_AGE[4] = {
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.25, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0)};

BeetrootBlock::BeetrootBlock() : CropBlock(true)
{
    CropBlock::DerivedInit();
    memset(mStageIcons, 0, sizeof(mStageIcons));
}

AbstractProperty<int>* BeetrootBlock::getAgeProperty()
{
    return AGE;
}

int BeetrootBlock::getMaxAge()
{
    return 3;
}

Item* BeetrootBlock::getBaseSeedId()
{
    return Items::beetrootSeeds;
}

Item* BeetrootBlock::getBasePlantId()
{
    return Items::beetroot;
}

void BeetrootBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (random->nextInt(3) == 0)
    {
        checkAlive(level, pos, state);
        return;
    }
    CropBlock::tick(level, pos, state, random);
}

int BeetrootBlock::getBonemealAgeIncrease(Level* level)
{
    return CropBlock::getBonemealAgeIncrease(level) / 3;
}

BlockStateDefinition* BeetrootBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {AGE};
    return new BlockStateDefinition(this, properties, 1);
}

const AABB* BeetrootBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[state->getValue(getAgeProperty())];
}

TextureAtlasSprite* BeetrootBlock::getTexture(const Direction*, const BlockState* state)
{
    int age = Mth::clamp(state->getValue(AGE), 0, getMaxAge());
    return mStageIcons[age];
}

void BeetrootBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < getMaxAge() + 1; ++i)
        mStageIcons[i] = iconRegister->registerIcon(L"beetroots_stage_" + _toString<int>(i));
}
