#include "game/world/block/GrassBlock.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <string>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* GrassBlock::SNOWY = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"snowy"));

GrassBlock::GrassBlock() : Block(Material::GRASS)
{
    DerivedInit();
    setTicking(true);
}

bool GrassBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool GrassBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

bool GrassBlock::randomWalk(Level* level, Random* random, BlockPos& pos, int distance)
{
    const int steps = distance / 16;
    for (int i = 0; i < steps; ++i)
    {
        const int dx = random->nextInt(3) - 1;
        const int dyDirection = random->nextInt(3) - 1;
        const int dy = dyDirection * random->nextInt(3) / 2;
        const int dz = random->nextInt(3) - 1;
        pos = pos.offset(dx, dy, dz);
        if (level->getBlockState(pos.below())->getBlock() != Blocks::grass
            || level->getBlockState(pos)->isSolidBlockingCubeAndNotSignalSource())
            return false;
    }
    return steps > 0;
}

void GrassBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState*)
{
    BlockPos target = pos.above();
    for (int distance = 16; distance < 64; ++distance)
    {
        BlockPos candidate = target;
        if (!randomWalk(level, random, candidate, distance))
            continue;
        if (level->getBlockState(candidate)->getBlock()->mMaterial != Material::AIR)
            continue;

        if (random->nextInt(8) == 0)
        {
            FlowerBlock::Flower* flower = level->getBiome(candidate)->getRandomFlower(*random, candidate);
            Block* block = flower->getCategory()->getBlock();

            const BlockState* generatedState = block->defaultBlockState()->setValue(
                static_cast<FlowerBlock*>(block)->getFlowerProperty(), flower);
            if (block->canSurvive(level, candidate, generatedState))
            {
                level->setBlock(candidate, generatedState, Block::UPDATE_ALL, false);
            }
        }
        else
        {
            const BlockState* generatedState = Blocks::tallgrass->defaultBlockState()->setValue(
                TallGrassBlock::VARIANT, TallGrassBlock::Variant::GRASS);
            if (Blocks::tallgrass->canSurvive(level, candidate, generatedState))
            {
                level->setBlock(candidate, generatedState, Block::UPDATE_ALL, false);
            }
        }
    }
}

int GrassBlock::getColor() const
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Grass_Common);
}

int GrassBlock::getColor(const BlockState* state)
{
    return getColor();
}

int GrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int)
{
    return getColor(levelSource, pos, levelSource->getBlockState(pos));
}

int GrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState*)
{
    return BiomeColors::getAverageGrassColor(levelSource, pos);
}

void GrassBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random* random)
{
    if (level->mIsClientSide)
        return;

    BlockPos above = pos.above();
    int brightness = level->getRawBrightness(above);
    if (brightness < 4)
    {
        int lightBlock = std::max(level->getBlockState(above)->getLightBlock(),
                                  level->getExtraBlockState(above)->getLightBlock());
        if (lightBlock > 2)
            level->setBlock(pos, Blocks::dirt->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        return;
    }
    if (brightness < 9)
        return;

    for (int i = 0; i < 4; ++i)
    {
        int targetZ = pos.z + random->nextInt(3) - 1;
        int targetY = pos.y + random->nextInt(2);
        int targetX = pos.x + random->nextInt(3) - 1;
        BlockPos target(targetX, targetY, targetZ);
        Block* block = level->getBlock(target);
        if (target.y >= 0 && target.y < 256 && !level->hasChunkAt(target))
            return;
        if (block != Blocks::dirt)
            continue;
        if (Blocks::dirt->getBlockState(level->getBlockData(target.x, target.y, target.z))->getValue(DirtBlock::VARIANT)
            != DirtBlock::Variant::DIRT)
        {
            continue;
        }

        const BlockState* aboveState = level->getBlockState(target.above());
        const BlockState* aboveExtraState = level->getExtraBlockState(target.above());
        Block* aboveBlock = aboveState->getBlock();
        Block* aboveExtraBlock = aboveExtraState->getBlock();
        Material* aboveMaterial = aboveBlock->mMaterial;
        Material* aboveExtraMaterial = aboveExtraBlock->mMaterial;
        int lightBlock = std::max(aboveState->getLightBlock(), aboveExtraState->getLightBlock());
        if (level->getRawBrightness(target.above()) >= 4 && !aboveState->isSolid() && !aboveMaterial->isLiquid()
            && !aboveExtraMaterial->isLiquid() && lightBlock <= 2)
        {
            level->setBlock(target, Blocks::grass->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }
    }
}

Item* GrassBlock::getResource(const BlockState*, Random* random, int lootBonus)
{
    return Blocks::dirt->getResource(
        Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::DIRT), random, lootBonus);
}

int GrassBlock::getRenderLayer()
{
    return 0;
}

int GrassBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

unsigned int GrassBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    case 290:
    case 291:
    case 292:
    case 293:
    case 294:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
            && tooltipData.m_bCanPlace)
            return IDS_TOOLTIPS_TILL;
        break;
    case 256:
    case 269:
    case 273:
    case 277:
    case 284:
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
            && tooltipData.m_bCanPlace)
            return IDS_TOOLTIPS_DIG_PATH;
        break;
    case 351:
        if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return IDS_TOOLTIPS_GROW;
        }
        break;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool GrassBlock::waterSpreadCausesSpawn()
{
    return true;
}

BlockStateDefinition* GrassBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = {SNOWY};
    return new BlockStateDefinition(this, properties, 1);
}

bool GrassBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState*)
{
    int lightBlock = std::max(level->getBlockState(pos.above())->getLightBlock(),
                              level->getExtraBlockState(pos.above())->getLightBlock());
    if (lightBlock > 2 && level->getRawBrightness(pos.above()) < 4)
        return true;
    if (level->getRawBrightness(pos.above()) >= 9)
        return true;
    return false;
}

TextureAtlasSprite* GrassBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP)
        return mTopIcon;
    if (direction == Direction::DOWN)
        return Blocks::dirt->getTexture(direction);
    return mIcon;
}

TextureAtlasSprite* GrassBlock::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                           const BlockState* state)
{
    if (direction == Direction::UP)
        return mTopIcon;
    if (direction == Direction::DOWN)
        return Blocks::dirt->getTexture(direction);
    Block* aboveBlock = levelSource->getBlockState(pos.above())->getBlock();
    Material* aboveMaterial = aboveBlock->mMaterial;
    if (aboveMaterial == Material::TOP_SNOW || aboveMaterial == Material::SNOW)
        return mSnowSideIcon;
    return mIcon;
}

void GrassBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"grass_side");
    mTopIcon = iconRegister->registerIcon(L"grass_top");
    mSnowSideIcon = iconRegister->registerIcon(L"snow_side");
    mSideOverlayIcon = iconRegister->registerIcon(L"grass_side_overlay");
}

TextureAtlasSprite* GrassBlock::getSideTextureOverlay()
{
    return static_cast<GrassBlock*>(Blocks::grass)->mSideOverlayIcon;
}
