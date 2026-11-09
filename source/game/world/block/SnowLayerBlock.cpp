#include "game/world/block/SnowLayerBlock.hpp"
#include "game/core/Direction.hpp"

#include <algorithm>

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* SnowLayerBlock::LAYERS = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"layers", 1, 8));
const AABB* SnowLayerBlock::SNOW_AABB[9] = {
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.0, 1.0),  AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.25, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0),  AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.625, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.75, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0),
};

SnowLayerBlock::SnowLayerBlock() : FallingBlock(Material::TOP_SNOW)
{
    mBlockType = static_cast<EBlockType>(EBLOCK_TYPE_NONE + 10);
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(LAYERS, 1));
    setTicking(true);
    setThickness(0.13f);
}

const AABB* SnowLayerBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SNOW_AABB[state->getValue(LAYERS)];
}

bool SnowLayerBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);
    if (state->getBlock() == Blocks::snow_layer)
        return state->getValue(LAYERS) < 5;
    return state->getBlock()->isPathfindable(levelSource, pos);
}

bool SnowLayerBlock::isTopSolidBlocking(const BlockState* state)
{
    return state->getValue(LAYERS) == 8;
}
EBlockFaceShape SnowLayerBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                                  const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

const AABB* SnowLayerBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    int layers = state->getValue(LAYERS) - 1;
    const AABB* shape = state->getShape(levelSource, pos);
    return AABB::newTemp(shape->mMinX, shape->mMinY, shape->mMinZ, shape->mMaxX, (float)layers * 0.125f, shape->mMaxZ);
}

double SnowLayerBlock::getHeight(Level* level, int x, int y, int z)
{
    float height = (float)(level->getBlockState(BlockPos(x, y, z))->getValue(LAYERS) * 2 + 2);
    return height * 0.0625f;
}

bool SnowLayerBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool SnowLayerBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool SnowLayerBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const BlockState* below = level->getBlockState(pos.below());
    Block* block = below->getBlock();
    if (block == Blocks::ice || block == Blocks::barrier)
        return false;
    return below->getBlockFaceShape(level, pos.below(), Direction::UP) == BLOCK_FACE_SOLID
           || below->getMaterial() == Material::LEAVES || (block == this && below->getValue(LAYERS) == 8);
}

void SnowLayerBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                     const BlockPos& fromPos)
{
    if (fromPos.y < pos.y && pos.x == fromPos.x && pos.z == fromPos.z)
        FallingBlock::neighborChanged(state, level, pos, block, fromPos);
}

bool SnowLayerBlock::isWaterBlocking() const
{
    return false;
}
bool SnowLayerBlock::waterSpreadCausesSpawn()
{
    return true;
}
bool SnowLayerBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

bool SnowLayerBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!mayPlace(level, pos))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
        return false;
    }
    return true;
}

void SnowLayerBlock::playerDestroy(Level* level, boost::shared_ptr<Player>, const BlockPos& pos,
                                   const BlockState* state, boost::shared_ptr<BlockEntity>, not_null_ptr<ItemInstance>)
{
    Block::popResource(level, pos,
                       not_null_ptr<ItemInstance>(new ItemInstance(Items::snowball, state->getValue(LAYERS) + 1, 0)));
    level->removeBlock(pos);
}

Item* SnowLayerBlock::getResource(const BlockState*, Random*, int)
{
    return Items::snowball;
}
int SnowLayerBlock::getResourceCount(Random*)
{
    return 0;
}

int SnowLayerBlock::melt(Level* level, const BlockPos& pos, int amount)
{
    if (amount > 0)
    {
        const BlockState* state = level->getBlockState(pos);
        int layers = state->getValue(LAYERS);
        if (amount >= layers + 1)
        {
            level->removeBlock(pos);
            return 1;
        }
        const BlockState* snow = buildSnowBlock(level, pos, -amount, 1);
        level->setBlock(pos, snow, Block::UPDATE_ALL, false);
    }
    return 0;
}

void SnowLayerBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Block* above = level->getBlock(pos.above());
    if (above == Blocks::snow_layer || above == Blocks::snow)
        return;

    int amount = 0;
    if (level->getGameRules()->getBoolean(GameRules::RULE_WEATHER_CYCLE) && random->nextInt(16) == 0)
    {
        Biome* biome = level->getBiome(pos);
        if (!level->isSnowingAt(pos) && (level->getBrightness(pos) > 11.0f || biome->getTemperature(pos) > 0.15f))
        {
            int minLayers = biome->getMinSnowLayers();
            if (minLayers < 1 || level->calcSnowBlockDepth(pos, minLayers + 1) > minLayers)
                amount = 1;
        }
    }

    int blockLight = level->getBrightness(LightLayer::BLOCK, pos);
    if (blockLight > 11)
    {
        amount = std::max(amount, std::min(blockLight - 11, 8));
    }

    if (melt(level, pos, amount) == 0)
    {
        Block* below = level->getBlock(pos.below());
        if (below == Blocks::air || below->defaultBlockState()->getMaterial() == Material::WATER)
            FallingBlock::tick(level, pos, state, random);
    }
}

bool SnowLayerBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                      const Direction* direction)
{
    if (direction == Direction::UP)
        return true;
    const BlockState* neighbour = levelSource->getBlockState(pos.relative(direction));
    if (neighbour->getBlock() == this && neighbour->getValue(LAYERS) >= state->getValue(LAYERS))
        return false;
    return Block::shouldRenderFace(state, levelSource, pos, direction);
}

const BlockState* SnowLayerBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(LAYERS, (data & 7) + 1);
}

bool SnowLayerBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return levelSource->getBlockState(pos)->getValue(LAYERS) == 1;
}

int SnowLayerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(LAYERS) - 1;
}
bool SnowLayerBlock::hasInHandRenderOffset()
{
    return true;
}

BlockStateDefinition* SnowLayerBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {LAYERS};
    return new BlockStateDefinition(this, properties, 1);
}

bool SnowLayerBlock::shouldBlockTick(Level*, const BlockPos&, const BlockState*)
{
    return true;
}

const BlockState* SnowLayerBlock::buildSnowBlock(Level* level, const BlockPos& pos, int layers, int addLayers)
{
    Block* block = level->getBlock(pos);
    const BlockState* state = level->getBlockState(pos);
    if (block == Blocks::snow_layer)
    {
        if (addLayers != 0)
        {
            layers += state->getValue(LAYERS);
            if (layers > 8)
                layers = 8;
        }
        return state->setValue(LAYERS, layers);
    }
    return Blocks::snow_layer->defaultBlockState()->setValue(LAYERS, layers);
}
