#include "game/world/block/FireBlock.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"

#include <algorithm>
#include <cstring>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/IconRegister.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

const std::wstring FireBlock::TEXTURE_FIRST = L"fire_0";
const std::wstring FireBlock::TEXTURE_SECOND = L"fire_1";

AbstractProperty<int>* FireBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 15));
AbstractProperty<bool>* FireBlock::NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
AbstractProperty<bool>* FireBlock::EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
AbstractProperty<bool>* FireBlock::SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
AbstractProperty<bool>* FireBlock::WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));
AbstractProperty<bool>* FireBlock::UP = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"upper"));

FireBlock::FireBlock() : Block(Material::FIRE), mFlameOdds(), mBurnOdds()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(AGE, 0)
                             ->setValue(NORTH, false)
                             ->setValue(EAST, false)
                             ->setValue(SOUTH, false)
                             ->setValue(WEST, false)
                             ->setValue(UP, false));
    setTicking(true);
    memset(mFireIcons, 0, sizeof(mFireIcons));
}

int FireBlock::getFlameOdds(Block* block)
{
    boost::unordered_map<Block*, int>::const_iterator it = mFlameOdds.find(block);
    return it == mFlameOdds.end() ? 0 : it->second;
}

bool FireBlock::canBurn(LevelSource* levelSource, const BlockPos& pos)
{
    return getFlameOdds(levelSource->getBlockState(pos)->getBlock()) > 0;
}

const BlockState* FireBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                             const BlockPos& pos)
{
    if (levelSource->getBlockState(pos.below())->isTopSolidBlocking()
        || static_cast<FireBlock*>(Blocks::fire)->canBurn(levelSource, pos.below()))
        return defaultBlockState();
    AbstractProperty<bool>* northProperty = NORTH;
    return state->setValue(northProperty, canBurn(levelSource, pos.north()))
        ->setValue(EAST, canBurn(levelSource, pos.east()))
        ->setValue(SOUTH, canBurn(levelSource, pos.south()))
        ->setValue(WEST, canBurn(levelSource, pos.west()))
        ->setValue(UP, canBurn(levelSource, pos.above()));
}

void FireBlock::setFlammable(Block* block, int flameOdds, int burnOdds)
{
    mFlameOdds[block] = flameOdds;
    mBurnOdds[block] = burnOdds;
}

void FireBlock::staticCtor()
{
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::planks, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::double_wooden_slab, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::wooden_slab, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::fence_gate, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::spruce_fence_gate, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::birch_fence_gate, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::jungle_fence_gate, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::dark_oak_fence_gate, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::acacia_fence_gate, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::fence, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::spruce_fence, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::birch_fence, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::jungle_fence, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::dark_oak_fence, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::acacia_fence, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::oak_stairs, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::birch_stairs, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::spruce_stairs, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::jungle_stairs, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::acacia_stairs, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::dark_oak_stairs, 5, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::log, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::log2, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::leaves, 30, 60);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::leaves2, 30, 60);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::bookshelf, 30, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::tnt, 15, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::tallgrass, 60, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::double_plant, 60, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::yellow_flower, 60, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::red_flower, 60, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::deadbush, 60, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::wool, 30, 60);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::vine, 15, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::coal_block, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::dried_kelp_block, 30, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::hay_block, 60, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::carpet, 60, 20);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::sea_pickle, 15, 100);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::stripped_log_oak, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::stripped_log_spruce, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::stripped_log_birch, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::stripped_log_jungle, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::stripped_log_acacia, 5, 5);
    static_cast<FireBlock*>(Blocks::fire)->setFlammable(Blocks::stripped_log_dark_oak, 5, 5);
}

const AABB* FireBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

bool FireBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool FireBlock::isCubeShaped(const BlockState*)
{
    return false;
}
int FireBlock::getResourceCount(Random*)
{
    return 0;
}
int FireBlock::getTickDelay(Level*)
{
    return 30;
}

int FireBlock::getFireOdds(Level* level, const BlockPos& pos)
{
    if (!level->isEmptyBlock(pos))
        return 0;
    int odds = 0;
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        odds = std::max(getFlameOdds(level->getBlockState(pos.relative(*it))->getBlock()), odds);
    }
    return odds;
}

bool FireBlock::isNearRain(Level* level, const BlockPos& pos)
{
    return level->isRainingAt(pos) || level->isRainingAt(pos.west()) || level->isRainingAt(pos.east())
           || level->isRainingAt(pos.north()) || level->isRainingAt(pos.south());
}

bool FireBlock::isValidFireLocation(Level* level, const BlockPos& pos)
{
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
        if (canBurn(level, pos.relative(*it)))
            return true;
    return false;
}

int FireBlock::getBurnOdd(Block* block)
{
    boost::unordered_map<Block*, int>::const_iterator it = mBurnOdds.find(block);
    return it == mBurnOdds.end() ? 0 : it->second;
}

void FireBlock::checkBurnOut(Level* level, const BlockPos& pos, int odds, Random* random, int age)
{
    int burnOdd = getBurnOdd(level->getBlockState(pos)->getBlock());
    if (random->nextInt(odds) >= burnOdd)
        return;

    const BlockState* burning = level->getBlockState(pos);
    if (random->nextInt(age + 10) < 5 && !level->isRainingAt(pos)
        && gApp.GetGameHostOption(eGameHostOption_FireSpreads))
    {
        int newAge = age + random->nextInt(5) / 4;
        if (newAge > 15)
            newAge = 15;
        level->setBlock(pos, defaultBlockState()->setValue(AGE, newAge), Block::UPDATE_ALL, false);
    }
    else
    {
        level->removeBlock(pos);
    }

    if (burning->getBlock() == Blocks::tnt)
    {
        Blocks::tnt->destroy(level, pos, burning->setValue(TntBlock::EXPLODE, true));
    }
}

void FireBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!level->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK))
        return;

    if (!level->mIsClientSide && !gMinecraftServer->getPlayers()->isTrackingBlock(pos, level->mDimension))
    {
        level->addToTickNextTick(pos, this, getTickDelay(level) * 5);
        return;
    }

    if (!mayPlace(level, pos))
        level->removeBlock(pos);

    Block* below = level->getBlockState(pos.below())->getBlock();
    bool infiniburn = below == Blocks::netherrack || below == Blocks::magma;
    if (dynamic_cast<TheEndDimension*>(level->mDimension) != NULL && below == Blocks::bedrock)
        infiniburn = true;

    int age = state->getValue(AGE);
    if (!infiniburn && level->isRaining() && isNearRain(level, pos))
    {
        level->removeBlock(pos);
        return;
    }

    if (age < 15)
    {
        state = state->setValue(AGE, age + random->nextInt(3) / 2);
        level->setBlock(pos, state, Block::UPDATE_INVISIBLE, false);
    }
    level->addToTickNextTick(pos, this, getTickDelay(level) + random->nextInt(10));

    if (!infiniburn)
    {
        if (!isValidFireLocation(level, pos)
            || level->getExtraBlockState(pos.below())->getMaterial() == Material::WATER)
        {
            if (!level->getBlockState(pos.below())->isTopSolidBlocking() || age > 3)
                level->removeBlock(pos);
            return;
        }
        if (!canBurn(level, pos.below()) && age == 15 && random->nextInt(4) == 0)
        {
            level->removeBlock(pos);
            return;
        }
    }

    bool humid = level->isHumidAt(pos);
    int humidity = humid ? -50 : 0;
    checkBurnOut(level, pos.east(), humidity + 300, random, age);
    checkBurnOut(level, pos.west(), humidity + 300, random, age);
    checkBurnOut(level, pos.below(), humidity + 250, random, age);
    checkBurnOut(level, pos.above(), humidity + 250, random, age);
    checkBurnOut(level, pos.north(), humidity + 300, random, age);
    checkBurnOut(level, pos.south(), humidity + 300, random, age);

    if (!gApp.GetGameHostOption(eGameHostOption_FireSpreads))
        return;

    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            for (int y = -1; y <= 4; ++y)
            {
                if (x == 0 && y == 0 && z == 0)
                    continue;
                int chance = 100;
                if (y > 1)
                    chance = y * 100;
                BlockPos target = pos.offset(x, y, z);
                int fireOdds = getFireOdds(level, target);
                if (fireOdds <= 0)
                    continue;
                int odds = (fireOdds + 40 + level->getDifficulty()->getId() * 7) / (age + 30);
                if (humid)
                    odds /= 2;
                if (odds < 1 || random->nextInt(chance) > odds || (level->isRaining() && isNearRain(level, target)))
                    continue;
                int newAge = age + random->nextInt(5) / 4;
                if (newAge > 15)
                    newAge = 15;
                level->setBlock(target, state->setValue(AGE, newAge), Block::UPDATE_ALL, false);
            }
        }
    }
}

bool FireBlock::canInstantlyTick()
{
    return false;
}
bool FireBlock::mayPick()
{
    return false;
}
bool FireBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->isTopSolidBlocking() || isValidFireLocation(level, pos);
}

void FireBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->getBlockState(pos.below())->isTopSolidBlocking() && !isValidFireLocation(level, pos))
        level->removeBlock(pos);
}

void FireBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if ((level->mDimension->getType() == DimensionType::NETHER
         || level->mDimension->getType() == DimensionType::OVERWORLD)
        && static_cast<PortalBlock*>(Blocks::portal)->trySpawnPortal(level, pos, true))
    {
        return;
    }
    if (!level->getBlockState(pos.below())->isTopSolidBlocking() && !isValidFireLocation(level, pos))
    {
        level->removeBlock(pos);
        return;
    }
    level->addToTickNextTick(pos, this, getTickDelay(level) + level->mRandom->nextInt(10));
}

void FireBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (random->nextInt(24) == 0)
        level->playLocalSound(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f, SoundEvents::FIRE_AMBIENT,
                              SoundSource::eSoundSource_Blocks, 1.0f + random->nextFloat(),
                              random->nextFloat() * 0.7f + 0.3f, false, 16.0f);

    if (level->getBlockState(pos.below())->isTopSolidBlocking()
        || static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.below()))
    {
        for (int i = 0; i < 3; ++i)
        {
            double x = pos.x + random->nextDouble();
            double y = pos.y + random->nextDouble() * 0.5 + 0.5;
            double z = pos.z + random->nextDouble();
            level->addParticle(PARTICLE_LARGE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }
    else
    {
        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.west()))
        {
            for (int i = 0; i < 2; ++i)
            {
                double x = pos.x + random->nextDouble() * 0.1f;
                double y = pos.y + random->nextDouble();
                double z = pos.z + random->nextDouble();
                level->addParticle(PARTICLE_LARGE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }

        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.east()))
        {
            for (int i = 0; i < 2; ++i)
            {
                double x = pos.x + 1 - random->nextDouble() * 0.1f;
                double y = pos.y + random->nextDouble();
                double z = pos.z + random->nextDouble();
                level->addParticle(PARTICLE_LARGE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }

        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.north()))
        {
            for (int i = 0; i < 2; ++i)
            {
                double x = pos.x + random->nextDouble();
                double y = pos.y + random->nextDouble();
                double z = pos.z + random->nextDouble() * 0.1f;
                level->addParticle(PARTICLE_LARGE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }

        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.south()))
        {
            for (int i = 0; i < 2; ++i)
            {
                double x = pos.x + random->nextDouble();
                double y = pos.y + random->nextDouble();
                double z = pos.z + 1 - random->nextDouble() * 0.1f;
                level->addParticle(PARTICLE_LARGE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }

        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.above()))
        {
            for (int i = 0; i < 2; ++i)
            {
                double x = pos.x + random->nextDouble();
                double y = pos.y + 1 - random->nextDouble() * 0.1f;
                double z = pos.z + random->nextDouble();
                level->addParticle(PARTICLE_LARGE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }
    }
}

const MaterialColor* FireBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::FIRE;
}
int FireBlock::getRenderLayer()
{
    return 0;
}
const BlockState* FireBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(AGE, data);
}
int FireBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}
bool FireBlock::waterSpreadCausesSpawn()
{
    return true;
}
bool FireBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* FireBlock::createBlockStateDefinition()
{
    static const Property* properties[6] = {AGE, NORTH, EAST, SOUTH, WEST, UP};
    return new BlockStateDefinition(this, properties, 6);
}

EBlockFaceShape FireBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* FireBlock::getTexture(const Direction*, const BlockState*)
{
    return mFireIcons[0];
}
TextureAtlasSprite* FireBlock::getTextureLayer(int layer)
{
    return mFireIcons[layer];
}

void FireBlock::registerIcons(IconRegister* iconRegister)
{
    mFireIcons[0] = iconRegister->registerIcon(TEXTURE_FIRST);
    mFireIcons[1] = iconRegister->registerIcon(TEXTURE_SECOND);
}

EBlockRenderShape FireBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_FIRE;
}
