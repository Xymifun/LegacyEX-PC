#include "game/world/block/ChorusFlowerBlock.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <cstdlib>

static AbstractProperty<int>* CHORUS_AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 5));

static inline AABB* createFlowerShape(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
{
    return AABB::newPermanent(minX, minY, minZ, maxX, maxY, maxZ)->divideInternalsBy(16.0);
}

static const AABB* FLOWER_UP_SHAPE = createFlowerShape(2.0, 14.0, 2.0, 14.0, 16.0, 14.0);
static const AABB* FLOWER_DOWN_SHAPE = createFlowerShape(2.0, 0.0, 2.0, 14.0, 14.0, 14.0);
static const AABB* FLOWER_SOUTH_SHAPE = createFlowerShape(2.0, 2.0, 14.0, 14.0, 14.0, 16.0);
static const AABB* FLOWER_NORTH_SHAPE = createFlowerShape(2.0, 2.0, 0.0, 14.0, 14.0, 2.0);
static const AABB* FLOWER_WEST_SHAPE = createFlowerShape(0.0, 2.0, 2.0, 2.0, 14.0, 14.0);
static const AABB* FLOWER_EAST_SHAPE = createFlowerShape(14.0, 2.0, 2.0, 16.0, 14.0, 14.0);

ChorusFlowerBlock::ChorusFlowerBlock() : Block(Material::PLANT, MaterialColor::COLOR_PURPLE)
{
    DerivedInit();
}

const AABB* ChorusFlowerBlock::getUpShape()
{
    return FLOWER_UP_SHAPE;
}
const AABB* ChorusFlowerBlock::getDownShape()
{
    return FLOWER_DOWN_SHAPE;
}
const AABB* ChorusFlowerBlock::getSouthShape()
{
    return FLOWER_SOUTH_SHAPE;
}
const AABB* ChorusFlowerBlock::getNorthShape()
{
    return FLOWER_NORTH_SHAPE;
}
const AABB* ChorusFlowerBlock::getEastShape()
{
    return FLOWER_EAST_SHAPE;
}
const AABB* ChorusFlowerBlock::getWestShape()
{
    return FLOWER_WEST_SHAPE;
}

void ChorusFlowerBlock::DerivedInit()
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(CHORUS_AGE, 0));
    setTicking(true);
    mCulledFace = NULL;
}

Item* ChorusFlowerBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

bool ChorusFlowerBlock::isDead(int age)
{
    return age >= 5;
}

bool ChorusFlowerBlock::allNeighborsEmpty(Level* level, const BlockPos& pos, const Direction* excluded)
{
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    const Direction** end = directions.data + directions.length;
    for (const Direction** it = directions.data; it != end; ++it)
    {
        if (*it != excluded && !level->isEmptyBlock(pos.relative(*it)))
        {
            return false;
        }
    }
    return true;
}

void ChorusFlowerBlock::placeGrownFlower(Level* level, const BlockPos& pos, int age)
{
    level->setBlock(pos, defaultBlockState()->setValue(CHORUS_AGE, age), Block::UPDATE_CLIENTS, false);
    level->levelEvent(LevelEvent::SOUND_CHORUS_GROW, pos, 0);
}

void ChorusFlowerBlock::placeDeadFlower(Level* level, const BlockPos& pos)
{
    level->setBlock(pos, defaultBlockState()->setValue(CHORUS_AGE, 5), Block::UPDATE_CLIENTS, false);
    level->levelEvent(LevelEvent::SOUND_CHORUS_DEATH, pos, 0);
}

void ChorusFlowerBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!canSurvive(level, pos, NULL))
    {
        level->destroyBlock(pos, true);
    }
    else
    {
        BlockPos above = pos.above();
        if (level->isEmptyBlock(above) && above.y < 256)
        {
            int age = state->getValue(CHORUS_AGE);
            if (!isDead(age) && random->nextInt(1) == 0)
            {
                bool growUp = false;
                bool rooted = false;
                const BlockState* belowState = level->getBlockState(pos.below());
                Block* belowBlock = belowState->getBlock();
                if (belowBlock == Blocks::end_stone)
                {
                    growUp = true;
                }
                else if (belowBlock == Blocks::chorus_plant)
                {
                    int stemHeight = 1;
                    for (int i = 0; i < 4; ++i)
                    {
                        Block* support = level->getBlock(pos.below(stemHeight + 1));
                        if (support == Blocks::chorus_plant)
                        {
                            ++stemHeight;
                        }
                        else
                        {
                            if (support == Blocks::end_stone)
                            {
                                rooted = true;
                            }
                            break;
                        }
                    }

                    int limit = 4;
                    if (rooted)
                    {
                        ++limit;
                    }
                    if (stemHeight < 2 || random->nextInt(limit) >= stemHeight)
                    {
                        growUp = true;
                    }
                }
                else if (belowState->getMaterial() == Material::AIR)
                {
                    growUp = true;
                }

                if (growUp && allNeighborsEmpty(level, above, NULL) && level->isEmptyBlock(pos.above(2)))
                {
                    level->setBlock(pos, Blocks::chorus_plant->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                    placeGrownFlower(level, above, age);
                }
                else if (age < 4)
                {
                    int branches = random->nextInt(4);
                    bool grew = false;
                    if (rooted)
                    {
                        ++branches;
                    }
                    for (int i = 0; i < branches; ++i)
                    {
                        const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(*random);
                        BlockPos branch = pos.relative(direction);
                        if (level->isEmptyBlock(branch) && level->isEmptyBlock(branch.below())
                            && allNeighborsEmpty(level, branch, direction->getOpposite()))
                        {
                            placeGrownFlower(level, branch, age + 1);
                            grew = true;
                        }
                    }
                    if (grew)
                    {
                        level->setBlock(pos, Blocks::chorus_plant->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                    }
                    else
                    {
                        placeDeadFlower(level, pos);
                    }
                }
                else if (age == 4)
                {
                    placeDeadFlower(level, pos);
                }
            }
        }
    }
}

bool ChorusFlowerBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool ChorusFlowerBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool ChorusFlowerBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && canSurvive(level, pos, NULL);
}

void ChorusFlowerBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, NULL))
    {
        level->addToTickNextTick(pos, this, 1);
    }
}

bool ChorusFlowerBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    const BlockState* below = level->getBlockState(pos.below());
    Block* block = below->getBlock();
    if (block == Blocks::chorus_plant || block == Blocks::end_stone)
    {
        return true;
    }
    if (below->getMaterial() == Material::AIR)
    {
        int neighbors = 0;
        arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
        const Direction** end = directions.data + directions.length;
        for (const Direction** it = directions.data; it != end; ++it)
        {
            const BlockState* neighbor = level->getBlockState(pos.relative(*it));
            if (neighbor->getBlock() == Blocks::chorus_plant)
            {
                ++neighbors;
            }
            else if (neighbor->getMaterial() != Material::AIR)
            {
                return false;
            }
        }
        return neighbors == 1;
    }
    return false;
}

void ChorusFlowerBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                      const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                      not_null_ptr<ItemInstance> tool)
{
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this))));
}

int ChorusFlowerBlock::getRenderLayer()
{
    return 0;
}

const BlockState* ChorusFlowerBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(CHORUS_AGE, data);
}

int ChorusFlowerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(CHORUS_AGE);
}

void ChorusFlowerBlock::growTreeRecursive(Level* level, const BlockPos& pos, Random* random, const BlockPos& origin,
                                          int radius, int depth)
{
    int height = random->nextInt(4) + 1;
    if (depth == 0)
    {
        ++height;
    }
    for (int i = 0; i < height; ++i)
    {
        BlockPos branch = pos.above(i + 1);
        if (!allNeighborsEmpty(level, branch, NULL))
        {
            return;
        }
        level->setBlock(branch, Blocks::chorus_plant->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
    bool grew = false;
    if (depth < 4)
    {
        int branches = random->nextInt(4);
        if (depth == 0)
        {
            ++branches;
        }
        for (int i = 0; i < branches; ++i)
        {
            const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(*random);
            BlockPos branch = pos.above(height).relative(direction);
            if (abs(branch.x - origin.x) < radius && abs(branch.z - origin.z) < radius && level->isEmptyBlock(branch)
                && level->isEmptyBlock(branch.below()) && allNeighborsEmpty(level, branch, direction->getOpposite()))
            {
                grew = true;
                level->setBlock(branch, Blocks::chorus_plant->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                growTreeRecursive(level, branch, random, origin, radius, depth + 1);
            }
        }
    }
    if (!grew)
    {
        level->setBlock(pos.above(height), Blocks::chorus_flower->defaultBlockState()->setValue(CHORUS_AGE, 5),
                        Block::UPDATE_CLIENTS, false);
    }
}

void ChorusFlowerBlock::generatePlant(Level* level, BlockPos pos, Random* random, int radius)
{
    level->setBlock(pos, Blocks::chorus_plant->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    growTreeRecursive(level, pos, random, pos, radius, 0);
}

void ChorusFlowerBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"chorus_flower");
    mDeadIcon = iconRegister->registerIcon(L"chorus_flower_dead");
    mPlantIcon = iconRegister->registerIcon(L"chorus_flower_plant");
}

TextureAtlasSprite* ChorusFlowerBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::DOWN)
    {
        return mPlantIcon;
    }
    return isDead(state->getValue(CHORUS_AGE)) ? mDeadIcon : mIcon;
}

EBlockRenderShape ChorusFlowerBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_CHORUS_FLOWER;
}
bool ChorusFlowerBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction* direction)
{
    return direction != mCulledFace;
}
void ChorusFlowerBlock::setCulledFace(const Direction* direction)
{
    mCulledFace = direction;
}
bool ChorusFlowerBlock::isWaterBlocking() const
{
    return false;
}
bool ChorusFlowerBlock::waterSpreadCausesSpawn()
{
    return true;
}
bool ChorusFlowerBlock::canContainLiquid(const BlockState*) const
{
    return false;
}
bool ChorusFlowerBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType)
{
    return false;
}
not_null_ptr<ItemInstance> ChorusFlowerBlock::getSilkTouchItemInstance(const BlockState*)
{
    return ItemInstance::EMPTY;
}

BlockStateDefinition* ChorusFlowerBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {CHORUS_AGE};
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape ChorusFlowerBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
