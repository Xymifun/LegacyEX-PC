#include "game/world/block/CakeBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/FoodConstants.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

static AbstractProperty<int>* BITES = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"bites", 0, 6));
static const AABB* SHAPES[7] = {AABB::newPermanent(0.0625, 0.0, 0.0625, 0.9375, 0.5, 0.9375),
                                AABB::newPermanent(0.1875, 0.0, 0.0625, 0.9375, 0.5, 0.9375),
                                AABB::newPermanent(0.3125, 0.0, 0.0625, 0.9375, 0.5, 0.9375),
                                AABB::newPermanent(0.4375, 0.0, 0.0625, 0.9375, 0.5, 0.9375),
                                AABB::newPermanent(0.5625, 0.0, 0.0625, 0.9375, 0.5, 0.9375),
                                AABB::newPermanent(0.6875, 0.0, 0.0625, 0.9375, 0.5, 0.9375),
                                AABB::newPermanent(0.8125, 0.0, 0.0625, 0.9375, 0.5, 0.9375)};

CakeBlock::CakeBlock() : Block(Material::CAKE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(BITES, 0));
    setTicking(true);
}

const AABB* CakeBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPES[state->getValue(BITES)];
}

bool CakeBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool CakeBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool CakeBlock::TestUse()
{
    return false;
}

bool CakeBlock::eat(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Player>& player)
{
    if (!player->canEat(false))
        return false;
    player->getFoodData()->eat(2, FoodConstants::FOOD_SATURATION_POOR);
    int bites = state->getValue(BITES);
    if (bites < 6)
    {
        level->setBlock(pos, state->setValue(BITES, bites + 1), Block::UPDATE_ALL, false);
    }
    else
        level->removeBlock(pos);
    return true;
}

bool CakeBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                    InteractionHand::EInteractionHand hand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide || !player->canEat(false))
        return false;
    if (level->mIsClientSide)
    {
        not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
        return eat(level, pos, state, player) || item->isEmpty();
    }
    return eat(level, pos, state, player);
}

bool CakeBlock::mayPlace(Level* level, const BlockPos& pos)
{
    if (Block::mayPlace(level, pos))
        return canSurvive(level, pos, NULL);
    return false;
}

void CakeBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, NULL))
        level->removeBlock(pos);
}

bool CakeBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return level->getBlockState(pos.below())->getMaterial()->isSolid();
}

int CakeBlock::getResourceCount(Random*)
{
    return 0;
}
Item* CakeBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

not_null_ptr<ItemInstance> CakeBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::cake));
}

int CakeBlock::getRenderLayer()
{
    return 0;
}

const BlockState* CakeBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(BITES, data);
}

int CakeBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(BITES);
}

unsigned int CakeBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_pPlayer->mAbilities.mInstabuild && tooltipData.m_pPlayer->getFoodData()->needsFood()
        && tooltipData.m_bUseBlock && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_EAT;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* CakeBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {BITES};
    return new BlockStateDefinition(this, properties, 1);
}

int CakeBlock::getAnalogOutputSignal(const BlockState* state, Level*, const BlockPos&)
{
    return 14 - state->getValue(BITES) * 2;
}

bool CakeBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

EBlockFaceShape CakeBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* CakeBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
        return mIcons[0];
    if (direction == Direction::DOWN)
        return mIcons[1];
    if (state->getValue(BITES) > 0 && direction == Direction::WEST)
        return mIcons[2];
    return mIcon;
}

void CakeBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"cake_side");
    mIcons[2] = iconRegister->registerIcon(L"cake_inner");
    mIcons[0] = iconRegister->registerIcon(L"cake_top");
    mIcons[1] = iconRegister->registerIcon(L"cake_bottom");
}
