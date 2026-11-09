#include "game/world/block/LiquidBlock.hpp"
#include "game/core/MemSect.hpp"

#include "game/core/BubbleColumnSegment.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/util/Math.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/StairBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"

#include <cmath>

AbstractProperty<int>* LiquidBlock::LEVEL = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"level", 0, 15));

const std::wstring LiquidBlock::TEXTURE_LAVA_STILL = L"lava";
const std::wstring LiquidBlock::TEXTURE_WATER_STILL = L"water";
const std::wstring LiquidBlock::TEXTURE_WATER_FLOW = L"water_flow";
const std::wstring LiquidBlock::TEXTURE_LAVA_FLOW = L"lava_flow";

LiquidBlock::LiquidBlock(Material* material) : Block(material)
{
    mBlockType = EBLOCK_TYPE_LIQUID;
    setTicking(true);
}

bool LiquidBlock::isLiquidBlock()
{
    return true;
}

float LiquidBlock::getHeightFromData(int data)
{
    if (data > 7)
        data = 0;
    return (float)(data + 1) / 9.0f;
}

unsigned short LiquidBlock::getLiquidElement(LevelSource* levelSource, const BlockPos& pos)
{
    unsigned short element;
    unsigned short alternate;
    levelSource->getElementAndExtraElement(pos.x, pos.y, pos.z, &element, &alternate);
    if (alternate == 0)
        return element;
    return alternate;
}

void LiquidBlock::DerivedInit()
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(LEVEL, 0));
}

const AABB* LiquidBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return FULL_BLOCK_SHAPE;
}

const AABB* LiquidBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool LiquidBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return mMaterial != Material::LAVA;
}

int LiquidBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int)
{
    return getColor(levelSource, pos, static_cast<const BlockState*>(NULL));
}

int LiquidBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState*)
{
    if (mMaterial != Material::WATER)
    {
        const int nonWaterColor = 0xffffff;
        return nonWaterColor;
    }
    return BiomeColors::getAverageWaterColor(levelSource, pos);
}

bool LiquidBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

int LiquidBlock::getDepth(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = Block::getStateByElementId(getLiquidElement(levelSource, pos));
    if (state->getMaterial() != mMaterial)
        return -1;
    return state->getValue(LEVEL);
}

int LiquidBlock::getRenderedDepth(LevelSource* levelSource, const BlockPos& pos)
{
    int depth = getDepth(levelSource, pos);
    if (depth >= 8)
        depth = 0;
    return depth;
}

bool LiquidBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool LiquidBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool LiquidBlock::mayPick(const BlockState* state, bool includeLiquid)
{
    return includeLiquid && state->getValue(LEVEL) == 0;
}

bool LiquidBlock::isSolidFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = Block::getStateByElementId(getLiquidElement(levelSource, pos));
    Block* block = state->getBlock();
    Material* material = state->getMaterial();
    if (material == mMaterial)
        return false;
    if (direction == Direction::UP)
        return true;
    if (material == Material::ICE)
        return false;
    bool attachable = Block::isExceptionBlockForAttachingPlusPiston(block)
                      || (block != NULL && dynamic_cast<StairBlock*>(block) != NULL);
    return !attachable && state->getBlockFaceShape(levelSource, pos, direction) == BLOCK_FACE_SOLID;
}

bool LiquidBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                   const Direction* direction)
{
    if (Block::getStateByElementId(getLiquidElement(levelSource, pos.relative(direction)))->getMaterial() == mMaterial)
        return false;
    if (direction == Direction::UP)
        return true;

    Block* block = levelSource->getBlock(pos);
    if (StairBlock::isStairs(Block::getId(block))
        && !block->liquidCanFlowIntoFromDirection(levelSource, pos, direction))
        return false;

    BlockPos adjacent = pos.relative(direction);
    int adjacentId = levelSource->getBlockId(adjacent.x, adjacent.y, adjacent.z);
    if (adjacentId == 208 || adjacentId == 272)
        return false;
    return Block::shouldRenderFace(state, levelSource, pos, direction);
}

bool LiquidBlock::shouldRenderBackwardUpFace(LevelSource* levelSource, const BlockPos& pos)
{
    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            const BlockState* state = Block::getStateByElementId(getLiquidElement(levelSource, pos.offset(x, 0, z)));
            if (state->getMaterial() != mMaterial && !state->isSolid() && state->getBlock() != Blocks::ice)
                return true;
        }
    }
    return false;
}

EBlockRenderShape LiquidBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_WATER;
}

Item* LiquidBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

int LiquidBlock::getResourceCount(Random*)
{
    return 0;
}

Vec3* LiquidBlock::getFlow(LevelSource* levelSource, const BlockPos& pos, const BlockState*)
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    int depth = getRenderedDepth(levelSource, pos);
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        BlockPos adjacent = pos.relative(direction);
        int adjacentDepth = getRenderedDepth(levelSource, adjacent);
        if (adjacentDepth < 0)
        {
            if (!levelSource->getBlockState(adjacent)->getBlock()->isWaterBlocking())
            {
                adjacentDepth = getRenderedDepth(levelSource, adjacent.below());
                if (adjacentDepth >= 0)
                {
                    int difference = adjacentDepth - (depth - 8);
                    x += direction->getStepX() * difference;
                    y += direction->getStepY() * difference;
                    z += direction->getStepZ() * difference;
                }
            }
        }
        else
        {
            int difference = adjacentDepth - depth;
            x += direction->getStepX() * difference;
            y += direction->getStepY() * difference;
            z += direction->getStepZ() * difference;
        }
    }
    return Vec3::newTemp(x, y, z)->normalize();
}

Vec3* LiquidBlock::handleEntityInside(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, Vec3* motion)
{
    return motion->add(getFlow(level, pos, level->getBlockState(entity->mTickCache, pos)));
}

void LiquidBlock::entityInside(Level* level, const BlockPos& pos, const BlockState*,
                               const boost::shared_ptr<Entity>& entity)
{
    const BlockState* below = level->getBlockState(entity->mTickCache, pos.below());
    if (below->getBlock() != Blocks::bubble_column_segment)
        return;

    bool drag = below->getValue(BubbleColumnSegment::DRAG);
    entity->onAboveBubbleCol(drag);
    if (level->mIsClientSide || level->mRandom->nextInt(4) == 0)
        return;

    ServerLevel* serverLevel = static_cast<ServerLevel*>(level);
    for (int i = 0; i < 2; ++i)
    {
        float x = static_cast<float>(pos.x) + level->mRandom->nextFloat();
        float z = static_cast<float>(pos.z) + level->mRandom->nextFloat();
        serverLevel->sendParticles(PARTICLE_SPLASH, x, pos.y + 1, z, 1, 0.0, 0.0, 0.0, 1.0, arrayWithLength<int>());
        x = static_cast<float>(pos.x) + level->mRandom->nextFloat();
        z = static_cast<float>(pos.z) + level->mRandom->nextFloat();
        serverLevel->sendParticles(PARTICLE_BUBBLE, x, pos.y + 1, z, 1, 0.0, 0.01, 0.0, 0.2, arrayWithLength<int>());
    }
}

int LiquidBlock::getTickDelay(Level* level)
{
    if (mMaterial == Material::WATER)
        return 5;
    if (mMaterial == Material::LAVA)
        return level->mDimension->isHasCeiling() ? 10 : 30;
    return 0;
}

int LiquidBlock::getLightColor(const BlockState*, LevelSource* levelSource, const BlockPos& pos, int lightValue)
{
    int color = levelSource->getLightColor(pos, 0, lightValue);
    int above = levelSource->getLightColor(pos.above(), 0, lightValue);
    int red = color & 255;
    int blue = above & 255;
    if (red < blue)
        red = blue;
    int green = (color >> 16) & 255;
    int aboveGreen = (above >> 16) & 255;
    if (aboveGreen < green)
        aboveGreen = green;
    return red | (aboveGreen << 16);
}

int LiquidBlock::getRenderLayer()
{
    return mMaterial == Material::WATER ? 2 : 0;
}

void LiquidBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    double x = pos.x;
    double y = pos.y;
    double z = pos.z;

    if (mMaterial == Material::WATER)
    {
        if (static_cast<unsigned int>(state->getValue(LEVEL) - 1) < 7)
        {
            if (random->nextInt(64) == 0)
            {
                level->playLocalSound(x + 0.5, y + 0.5, z + 0.5, SoundEvents::WATER_AMBIENT,
                                      SoundSource::eSoundSource_Blocks, random->nextFloat() * 0.25f + 0.75f,
                                      random->nextFloat() * 1.0f + 0.5f, false, 16.0f);
            }
        }
        else if (random->nextInt(25) == 0)
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            for (int i = 0; i < 4; ++i)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[i];
                if (player && player->isUnderLiquid(mMaterial) && player->distanceToSqr(pos) < 64.0)
                {
                    level->addParticle(PARTICLE_SUSPENDED, x + random->nextFloat(), y + random->nextFloat(),
                                       z + random->nextFloat(), 0.0, 0.0, 0.0, arrayWithLength<int>());
                    break;
                }
            }
        }

        const BlockState* below = level->getBlockState(pos.below());
        if (below->getBlock() == Blocks::bubble_column_segment)
        {
            if (below->getValue(BubbleColumnSegment::DRAG))
            {
                level->addAlwaysVisibleParticle(ePARTICLE_CURRENT_DOWN, pos.x + 0.3, pos.y + 0.8, pos.z + 0.3, 0.0, 0.0,
                                                0.0, arrayWithLength<int>());
            }
            else
            {
                double bubbleX = x + random->nextFloat();
                double bubbleZ = z + random->nextFloat();
                level->addAlwaysVisibleParticle(ePARTICLE_BUBBLE_POP, bubbleX, y + 1.05, bubbleZ, 0.0, 0.0, 0.0,
                                                arrayWithLength<int>());
            }
        }
    }

    if (mMaterial == Material::LAVA && level->getBlockState(pos.above())->getBlock()->mMaterial == Material::AIR
        && !level->getBlockState(pos.above())->isSolidRender())
    {
        if (random->nextInt(100) == 0)
        {
            double lavaX = x + random->nextFloat();
            double lavaY = y + state->getShape(level, pos)->mMaxY;
            double lavaZ = z + random->nextFloat();
            level->addParticle(PARTICLE_LAVA, lavaX, lavaY, lavaZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
            level->playLocalSound(lavaX, lavaY, lavaZ, SoundEvents::LAVA_POP, SoundSource::eSoundSource_Blocks,
                                  random->nextFloat() * 0.2f + 0.2f, random->nextFloat() * 0.15f + 0.9f, false, 16.0f);
        }
        if (random->nextInt(200) == 0)
        {
            level->playLocalSound(x, y, z, SoundEvents::LAVA_AMBIENT, SoundSource::eSoundSource_Blocks,
                                  random->nextFloat() * 0.2f + 0.2f, random->nextFloat() * 0.15f + 0.9f, false, 16.0f);
        }
    }

    if (random->nextInt(10) == 0 && level->getBlockState(pos.below())->isTopSolidBlocking())
    {
        Material* material = level->getBlockState(pos.below(2))->getBlock()->mMaterial;
        if (!material->blocksMotion() && !material->isLiquid())
        {
            double dripX = x + random->nextFloat();
            double dripY = y - 1.05;
            double dripZ = z + random->nextFloat();
            if (mMaterial == Material::WATER)
                level->addParticle(PARTICLE_DRIP_WATER, dripX, dripY, dripZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
            else
                level->addParticle(PARTICLE_DRIP_LAVA, dripX, dripY, dripZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }
}

LiquidBlock* LiquidBlock::getDynamicLiquid(Material* material)
{
    if (material == Material::WATER)
        return static_cast<LiquidBlock*>(Blocks::flowing_water);
    if (material == Material::LAVA)
        return static_cast<LiquidBlock*>(Blocks::flowing_lava);
    return NULL;
}

double LiquidBlock::getSlopeAngle(LevelSource* levelSource, const BlockPos& pos, Material* material,
                                  const BlockState* state)
{
    LiquidBlock* liquid = getDynamicLiquid(material);
    Vec3* flow = liquid->getFlow(levelSource, pos, state);
    if (flow->x == 0.0 && flow->z == 0.0)
        return -1000.0;
    return std::atan2(flow->z, flow->x) - 1.5707963705062866;
}

void LiquidBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    updateLiquid(level, pos, state);
}

void LiquidBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || miniGame->shouldTickBlocks())
        updateLiquid(level, pos, state);
}

bool LiquidBlock::updateLiquid(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (mMaterial != Material::LAVA)
        return false;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !Minecraft::GetInstance()->GetMiniGame()->canLavaConvertWater())
        return false;
    for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
    {
        const Direction* direction = Direction::VALUES[i];
        if (direction == Direction::DOWN)
            continue;
        Block* neighborBlock = level->getLiquidBlock(pos.relative(direction));
        if (neighborBlock->mMaterial == Material::WATER)
        {
            int depth = state->getValue(LEVEL);
            if (depth != 0)
            {
                if (depth > 4)
                    return false;
                level->setBlockAndUpdate(pos, Blocks::cobblestone->defaultBlockState());
                fizz(level, pos);
                return true;
            }
            level->setBlockAndUpdate(pos, Blocks::obsidian->defaultBlockState());
            fizz(level, pos);
            return true;
        }
    }
    return false;
}

bool LiquidBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

void LiquidBlock::fizz(Level* level, const BlockPos& pos)
{
    double x = pos.x;
    double y = pos.y;
    double z = pos.z;
    MemSect(31);
    level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::LAVA_EXTINGUISH, SoundSource::eSoundSource_Blocks,
                     0.5f, (level->mRandom->nextFloat() - level->mRandom->nextFloat()) * 0.8f + 2.6f, 16.0f);
    MemSect(0);
    for (int i = 0; i < 8; ++i)
    {
        level->addParticle(PARTICLE_LARGE_SMOKE, x + Math::random(), y + 1.2, z + Math::random(), 0.0, 0.0, 0.0,
                           arrayWithLength<int>());
    }
}

const BlockState* LiquidBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(LEVEL, data);
}

int LiquidBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(LEVEL);
}

BlockStateDefinition* LiquidBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {LEVEL};
    return new BlockStateDefinition(this, properties, 1);
}

unsigned short LiquidBlock::getDynamicLiquidElementId(unsigned short elementId)
{
    unsigned short type = elementId >> 4;
    unsigned short data = elementId & 15;
    if (type == 9)
        return data | 0x80;
    if (type == 11)
        return data | 0xa0;
    return (type << 4) | data;
}

unsigned short LiquidBlock::getStaticLiquidElementId(unsigned short elementId)
{
    unsigned short type = elementId >> 4;
    unsigned short data = elementId & 15;
    if (type == 8)
        return data | 0x90;
    if (type == 10)
        return data | 0xb0;
    return (type << 4) | data;
}

void LiquidBlock::registerIcons(IconRegister* iconRegister)
{
    if (mMaterial == Material::LAVA)
    {
        mIcons[0] = iconRegister->registerIcon(TEXTURE_LAVA_STILL);
        mIcons[1] = iconRegister->registerIcon(TEXTURE_LAVA_FLOW);
    }
    else
    {
        mIcons[0] = iconRegister->registerIcon(TEXTURE_WATER_STILL);
        mIcons[1] = iconRegister->registerIcon(TEXTURE_WATER_FLOW);
    }
}

TextureAtlasSprite* LiquidBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::DOWN || direction == Direction::UP)
        return mIcons[0];
    return mIcons[1];
}

float LiquidBlock::getBlockLiquidHeight(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    int data = state->getValue(LEVEL);
    if ((data & 7) == 0 && levelSource->getBlockState(pos.above())->getMaterial() == Material::WATER)
        return 1.0f;
    return 1.0f - getHeightFromData(data);
}

float LiquidBlock::getLiquidHeight(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return (float)pos.y + getBlockLiquidHeight(state, levelSource, pos);
}

EBlockFaceShape LiquidBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void LiquidBlock::setLiquidElement(Level* level, const BlockPos& pos, unsigned short elementId, bool extraLayer,
                                   int flags)
{
    const BlockState* state = Block::getStateByElementId(elementId);
    if (extraLayer)
        level->setExtraBlock(pos, state, flags, false);
    else
        level->setBlock(pos, state, flags, false);
}

void LiquidBlock::trySpreadFire(Level* level, const BlockPos& pos, Random* random)
{
    if (!level->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK))
        return;

    Block* above = level->getBlock(pos.above());
    if (above->defaultBlockState()->getMaterial() != Material::AIR
        && !above->defaultBlockState()->getMaterial()->isFlammable())
        return;

    for (int i = 0; i < 5; ++i)
    {
        BlockPos candidate = pos;
        candidate = candidate.offset(random->nextInt(3) - 1, random->nextInt(3) - 1, random->nextInt(3) - 1);
        if (static_cast<unsigned int>(candidate.y) < 256 && !level->hasChunkAt(candidate))
            return;

        if (level->getBlock(candidate)->defaultBlockState()->getMaterial()->isFlammable())
        {
            for (unsigned int direction = 0; direction < Direction::VALUES.size(); ++direction)
            {
                BlockPos adjacent = candidate.relative(Direction::VALUES[direction]);
                if (level->getBlock(adjacent)->defaultBlockState()->getMaterial() == Material::AIR)
                {
                    level->setBlock(adjacent, Blocks::fire->defaultBlockState(),
                                    Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
                    return;
                }
            }
        }

        candidate = pos;
        candidate = candidate.offset(random->nextInt(3) - 1, random->nextInt(3) - 1, random->nextInt(3) - 1);
        if (static_cast<unsigned int>(candidate.y) < 256 && !level->hasChunkAt(candidate))
            return;

        if (level->getBlock(candidate)->defaultBlockState()->getMaterial()->isFlammable())
        {
            for (unsigned int direction = 0; direction < Direction::VALUES.size(); ++direction)
            {
                BlockPos adjacent = candidate.relative(Direction::VALUES[direction]);
                if (level->getBlock(adjacent)->defaultBlockState()->getMaterial() == Material::AIR)
                {
                    level->setBlock(adjacent, Blocks::fire->defaultBlockState(),
                                    Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
                    return;
                }
            }
        }
    }
}
