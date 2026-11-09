#include "game/world/block/MyceliumBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* MyceliumBlock::SNOWY = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"snowy"));

MyceliumBlock::MyceliumBlock() : Block(Material::GRASS, MaterialColor::COLOR_PURPLE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(SNOWY, false));
    setTicking(true);
}

const BlockState* MyceliumBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                 const BlockPos& pos)
{
    Block* aboveBlock = levelSource->getBlockState(pos.above())->getBlock();
    return state->setValue(SNOWY, aboveBlock == Blocks::snow || aboveBlock == Blocks::snow_layer);
}

void MyceliumBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random* random)
{
    if (level->mIsClientSide)
    {
        return;
    }

    int lightBlock = level->getBlockState(pos.above())->getLightBlock();
    int extraLightBlock = level->getExtraBlockState(pos.above())->getLightBlock();
    lightBlock = lightBlock < extraLightBlock ? extraLightBlock : lightBlock;

    if (level->getRawBrightness(pos.above()) < 4 && lightBlock > 2)
    {
        level->setBlockAndUpdate(
            pos, Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::DIRT));
        return;
    }

    int brightness = level->getRawBrightness(pos.above());
    if (brightness >= 9)
    {
        for (int i = 0; i < 4; ++i)
        {
            BlockPos target = pos.offset(random->nextInt(3) - 1, random->nextInt(5) - 3, random->nextInt(3) - 1);
            const BlockState* targetState = level->getBlockState(target);
            int targetLightBlock = level->getBlockState(pos.above())->getLightBlock();
            int targetExtraLightBlock = level->getExtraBlockState(pos.above())->getLightBlock();
            targetLightBlock = targetLightBlock < targetExtraLightBlock ? targetExtraLightBlock : targetLightBlock;
            if (targetState->getBlock() == Blocks::dirt
                && targetState->getValue(DirtBlock::VARIANT) == DirtBlock::Variant::DIRT)
            {
                if (level->getRawBrightness(target.above()) >= 4 && targetLightBlock <= 2)
                {
                    level->setBlockAndUpdate(target, defaultBlockState());
                }
            }
        }
    }
}

void MyceliumBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    Block::animateTick(state, level, pos, random);
    if (random->nextInt(10) == 0)
    {
        level->addParticle(PARTICLE_TOWN_AURA, (double)((float)pos.x + random->nextFloat()),
                           (double)((float)pos.y + 1.1f), (double)((float)pos.z + random->nextFloat()), 0.0, 0.0, 0.0,
                           arrayWithLength<int>());
    }
}

Item* MyceliumBlock::getResource(const BlockState*, Random* random, int lootBonus)
{
    return Blocks::dirt->getResource(
        Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::DIRT), random, lootBonus);
}

int MyceliumBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

BlockStateDefinition* MyceliumBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {SNOWY};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* MyceliumBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::dirt->getTexture(direction);
    }
    return mIcon;
}

TextureAtlasSprite* MyceliumBlock::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                              const BlockState* state)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::dirt->getTexture(direction);
    }
    Block* aboveBlock = levelSource->getBlockState(pos.above())->getBlock();
    Material* material = aboveBlock->mMaterial;
    if (material == Material::TOP_SNOW || material == Material::SNOW)
    {
        return mSnowIcon;
    }
    return mIcon;
}

void MyceliumBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"mycel_side");
    mTopIcon = iconRegister->registerIcon(L"mycel_top");
    mSnowIcon = iconRegister->registerIcon(L"snow_side");
}
