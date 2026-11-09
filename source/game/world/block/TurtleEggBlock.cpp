#include "game/world/block/TurtleEggBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Turtle.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
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
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

AbstractProperty<int>* TurtleEggBlock::EGG_COUNT = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"egg_count", 0, 3));
AbstractProperty<int>* TurtleEggBlock::HATCH_LEVEL = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"hatch_level", 0, 2));

static const AABB* EGG_ONE_AABB = AABB::newPermanent(0.3125, 0.0, 0.25, 0.625, 0.4375, 0.5625);
static const AABB* EGG_TWO_AABB = AABB::newPermanent(0.0625, 0.0, 0.4375, 0.3125, 0.3125, 0.6875);
static const AABB* EGG_THREE_AABB = AABB::newPermanent(0.6875, 0.0, 0.4375, 0.875, 0.25, 0.625);
static const AABB* EGG_FOUR_AABB = AABB::newPermanent(0.4375, 0.0, 0.625, 0.625, 0.1875, 0.8125);
static const AABB* ONE_EGG_SHAPE = AABB::newPermanent(0.3125, 0.0, 0.25, 0.625, 0.4375, 0.5625);
static const AABB* TWO_EGG_SHAPE = AABB::newPermanent(0.0625, 0.0, 0.25, 0.625, 0.4375, 0.6875);
static const AABB* THREE_EGG_SHAPE = AABB::newPermanent(0.0625, 0.0, 0.25, 0.875, 0.4375, 0.6875);
static const AABB* FOUR_EGG_SHAPE = AABB::newPermanent(0.0625, 0.0, 0.25, 0.875, 0.4375, 0.8125);

std::wstring TEXTURE_NAMES[3] = {L"turtle_egg_hatch_0", L"turtle_egg_hatch_1", L"turtle_egg_hatch_2"};
TextureAtlasSprite* TEXTURES[3];

TurtleEggBlock::TurtleEggBlock() : Block(Material::EGG, MaterialColor::COLOR_SILVER)
{
    DerivedInit();
    registerDefaultState(defaultBlockState()->setValue(EGG_COUNT, 0)->setValue(HATCH_LEVEL, 0));
    setTicking(true);
}

bool TurtleEggBlock::shouldUpdateHatchLevel(Level* level)
{
    float timeOfDay = level->getTimeOfDay(1.0f);
    if (timeOfDay < 0.69f && timeOfDay > 0.65f)
        return true;
    return level->mRandom->nextInt(500) == 0;
}

void TurtleEggBlock::_decreaseEggs(Level* level, const BlockPos& pos, int eggCount, bool crack, bool jump) const
{
    SoundEvent* sound;
    if (!crack)
        sound = SoundEvents::TURTLE_EGG_BREAK;
    else if (jump)
        sound = SoundEvents::TURTLE_EGG_HATCH;
    else
        sound = SoundEvents::TURTLE_EGG_CRACK;
    level->playSound(boost::shared_ptr<Player>(), pos, sound, SoundSource::eSoundSource_Blocks, 0.7f,
                     level->mRandom->nextFloat() * 0.2f + 0.9f, 16.0f);

    const BlockState* state = level->getBlockState(pos);
    level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getId(state->getBlock()));
    if (eggCount == 0)
    {
        level->removeBlock(pos);
        return;
    }
    level->setBlock(pos, state->setValue(EGG_COUNT, eggCount - 1), Block::UPDATE_ALL, false);
}

void TurtleEggBlock::_destroyEgg(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, int chance,
                                 bool crack, bool jump) const
{
    if (entity == NULL)
        return;
    bool isTurtle = eTYPE_DERIVED_FROM(EntityType::TURTLE, entity->GetType());
    if (!isTurtle && !level->mIsClientSide && level->mRandom->nextInt(chance) == 0)
    {
        _decreaseEggs(level, pos, level->getBlockState(pos)->getValue(EGG_COUNT), crack, jump);
    }
}

void TurtleEggBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    _destroyEgg(level, pos, entity, 100, false, true);
    Block::stepOn(level, pos, entity);
}

void TurtleEggBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    bool isZombie = eTYPE_DERIVED_FROM(EntityType::ZOMBIE, entity->GetType());
    if (!isZombie)
    {
        _destroyEgg(level, pos, entity, 3, false, true);
    }
    Block::fallOn(level, pos, entity, fallDistance);
}

void TurtleEggBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (shouldUpdateHatchLevel(level) && level->getBlockState(pos.below())->getBlock() == Blocks::sand)
    {
        int hatchLevel = state->getValue(HATCH_LEVEL);
        if (hatchLevel < 2)
        {
            level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TURTLE_EGG_CRACK,
                             SoundSource::eSoundSource_Blocks, 0.7f, level->mRandom->nextFloat() * 0.2f + 0.9f, 16.0f);
            level->setBlock(pos, state->setValue(HATCH_LEVEL, hatchLevel + 1), Block::UPDATE_CLIENTS, false);
        }
        else
        {
            level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TURTLE_EGG_HATCHED,
                             SoundSource::eSoundSource_Blocks, 0.7f, level->mRandom->nextFloat() * 0.2f + 0.9f, 16.0f);
            if (!level->mIsClientSide)
            {
                for (int i = 0; i <= state->getValue(EGG_COUNT); ++i)
                {
                    level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getId(state->getBlock()));
                    boost::shared_ptr<Turtle> turtle(new Turtle(level));
                    turtle->setAge(-24000);
                    turtle->setHomePos(pos);
                    turtle->moveTo((double)pos.x + (double)i * 0.2 + 0.3, (double)pos.y, (double)pos.z + 0.3, 0.0f,
                                   0.0f);
                    level->addEntity(turtle);
                }
            }
            level->removeBlock(pos);
        }
    }
    int delay = level->mRandom->nextInt(750, 1500);
    level->addToTickNextTick(pos, this, delay);
}

bool TurtleEggBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                       boost::shared_ptr<Player> player)
{
    int eggCount = state->getValue(EGG_COUNT);
    bool silkTouch = EnchantmentHelper::getItemEnchantmentLevel(Enchantments::SILK_TOUCH, player->getMainHandItem())
                     > 0;
    _decreaseEggs(level, pos, eggCount, silkTouch, false);
    return true;
}

Item* TurtleEggBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

bool TurtleEggBlock::mayPlace(Level* level, const BlockPos& pos)
{
    if (level->getBlockId(pos.x, pos.y, pos.z) == BlockId::TURTLE_EGG)
        return true;
    const BlockState* below = level->getBlockState(pos.below());
    Block* block = below->getBlock();
    if (below->canProvideSupport(Direction::UP, BLOCK_SUPPORT_CENTER) && Block::getId(block) != BlockId::MAGMA
        && !block->mIsFence)
        return true;
    return false;
}

bool TurtleEggBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    not_null_ptr<ItemInstance> item = player->getMainHandItem();
    if (!item->isEmpty() && item->getItem() == Item::byBlock(Blocks::turtle_egg))
    {
        if (level->mIsClientSide)
            return true;

        const BlockState* current = level->getBlockState(pos);
        if (current->getBlock() != Blocks::turtle_egg)
            return false;
        int eggCount = current->getValue(EGG_COUNT);
        if (eggCount < 3)
        {
            const BlockState* newState = current->setValue(EGG_COUNT, eggCount + 1);
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            if (level->setBlock(pos, newState, Block::UPDATE_ALL, false))
            {
                level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TURTLE_LAY_EGG,
                                 SoundSource::eSoundSource_Players, 0.5f, level->mRandom->nextFloat() * 0.2f + 0.9f,
                                 16.0f);
                if (level->getBlock(pos.below()) == Blocks::sand)
                    level->levelEvent(LevelEvent::PARTICLES_PLANT_GROWTH, pos, 15);
            }
            return true;
        }
    }
    return false;
}

bool TurtleEggBlock::TestUse(Level*, const BlockPos&, const BlockState* state, boost::shared_ptr<Player> player)
{
    int canUse = state->getValue(EGG_COUNT) != 3
                 && player->getMainHandItem()->getItem() == Item::byBlock(Blocks::turtle_egg);
    return canUse != 0;
}

unsigned int TurtleEggBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (TestUse(tooltipData.m_pLevel, tooltipData.m_blockPos, tooltipData.m_pBlockState, tooltipData.m_pPlayer))
        return IDS_TOOLTIPS_PLACE;
    return Block::GetInteractTooltip(tooltipData);
}

bool TurtleEggBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool TurtleEggBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void TurtleEggBlock::addCollisionAABBs(const BlockState* state, Level*, const BlockPos& pos, const AABB* entityBox,
                                       std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity>, bool)
{
    int eggCount = state->getValue(EGG_COUNT);
    if (eggCount >= 0)
        addCollisionAABB(pos, entityBox, collidingBoxes, EGG_ONE_AABB);
    if (eggCount >= 1)
        addCollisionAABB(pos, entityBox, collidingBoxes, EGG_TWO_AABB);
    if (eggCount >= 2)
        addCollisionAABB(pos, entityBox, collidingBoxes, EGG_THREE_AABB);
    if (eggCount >= 3)
        addCollisionAABB(pos, entityBox, collidingBoxes, EGG_FOUR_AABB);
}

const AABB* TurtleEggBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(EGG_COUNT))
    {
    case 0:
        return ONE_EGG_SHAPE;
    case 1:
        return TWO_EGG_SHAPE;
    case 2:
        return THREE_EGG_SHAPE;
    default:
        return FOUR_EGG_SHAPE;
    }
}

EBlockRenderShape TurtleEggBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TURTLE_EGG;
}
int TurtleEggBlock::getRenderLayer()
{
    return 0;
}
bool TurtleEggBlock::canInstantlyTick()
{
    return false;
}

void TurtleEggBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (!level->mIsClientSide && level->getBlockState(pos.below())->getBlock() == Blocks::sand)
        level->levelEvent(LevelEvent::PARTICLES_PLANT_GROWTH, pos, 15);
    level->addToTickNextTick(pos, this, 1000);
}

bool TurtleEggBlock::canContainLiquid(const BlockState*) const
{
    return true;
}
bool TurtleEggBlock::waterSpreadCausesSpawn()
{
    return false;
}
EPistonPushReaction TurtleEggBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_DESTROY;
}

int TurtleEggBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(EGG_COUNT) | state->getValue(HATCH_LEVEL) << 2;
}

const BlockState* TurtleEggBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(EGG_COUNT, data & 3)->setValue(HATCH_LEVEL, data >> 2 & 3);
}

TextureAtlasSprite* TurtleEggBlock::getTexture(const Direction*, const BlockState* state)
{
    return TEXTURES[state->getValue(HATCH_LEVEL)];
}

void TurtleEggBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 3; i++)
        TEXTURES[i] = iconRegister->registerIcon(TEXTURE_NAMES[i]);
}

BlockStateDefinition* TurtleEggBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {EGG_COUNT, HATCH_LEVEL};
    return new BlockStateDefinition(this, properties, 2);
}

bool TurtleEggBlock::isSilkTouchable()
{
    return true;
}
