#include "game/world/block/SaplingBlock.hpp"
#include "game/core/StringIds.hpp"
#include <typeinfo>
#include <cstring>
#include <boost/unordered_set.hpp>
#include "game/core/DyeColor.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/BirchFeature.hpp"
#include "game/world/generation/MegaJungleTreeFeature.hpp"
#include "game/world/generation/MegaPineTreeFeature.hpp"
#include "game/world/generation/RoofTreeFeature.hpp"
#include "game/world/generation/SavannaTreeFeature.hpp"
#include "game/world/generation/SpruceFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/item/ItemId.hpp"

EnumProperty<PlanksBlock::Variant*>* SaplingBlock::TYPE = NULL;
static AbstractProperty<int>* gSaplingStage = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"stage", 0, 1));
static const AABB* SAPLING_SHAPE = AABB::newPermanent(0.09999999403953552, 0.0, 0.09999999403953552, 0.8999999761581421,
                                                      0.800000011920929, 0.8999999761581421);
static unsigned int SAPLING_DESCRIPTION_IDS[6] = {IDS_TILE_SAPLING_OAK,    IDS_TILE_SAPLING_SPRUCE,
                                                  IDS_TILE_SAPLING_BIRCH,  IDS_TILE_SAPLING_JUNGLE,
                                                  IDS_TILE_SAPLING_ACACIA, IDS_TILE_SAPLING_DARKOAK};

static const std::wstring SAPLING_ICON_NAMES[6] = {L"sapling",        L"sapling_spruce", L"sapling_birch",
                                                   L"sapling_jungle", L"sapling_acacia", L"sapling_roofed_oak"};

SaplingBlock::SaplingBlock()
{
    DerivedInit();
    registerDefaultState(
        mStateDefinition->any()->setValue(TYPE, PlanksBlock::Variant::OAK)->setValue(gSaplingStage, 0));
    memset(mIcons, 0, sizeof(mIcons));
}

bool SaplingBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool SaplingBlock::isBonemealSuccess(Level* level, Random*, const BlockPos&, const BlockState*)
{
    bool success = true;
    if (!((GameMode*)Minecraft::GetInstance()->mGameMode)->isCreative())
        success = (double)level->mRandom->nextFloat() < 0.45;
    return success;
}

bool SaplingBlock::isSapling(Level* level, const BlockPos& pos, PlanksBlock::Variant* variant)
{
    const BlockState* state = level->getBlockState(pos);
    return state->getBlock() == this && state->getValue(TYPE) == variant;
}

bool SaplingBlock::isTwoByTwoSaplingOfType(Level* level, const BlockPos& pos, int x, int z,
                                           PlanksBlock::Variant* variant)
{
    return isSapling(level, pos.offset(x, 0, z), variant) && isSapling(level, pos.offset(x + 1, 0, z), variant)
           && isSapling(level, pos.offset(x, 0, z + 1), variant)
           && isSapling(level, pos.offset(x + 1, 0, z + 1), variant);
}

void SaplingBlock::growTree(Level* level, const BlockPos& pos, const BlockState* state, Random* random, bool bonemeal)
{
    AbstractTreeFeature* feature = random->nextInt(10) == 0
                                       ? static_cast<AbstractTreeFeature*>(new BigTreeFeature(true))
                                       : new TreeFeature(true);
    int dx = 0;
    int dz = 0;
    bool mega = false;
    switch (state->getValue(TYPE)->getData())
    {
    case PlanksBlock::Variant::eDataSpruce:
        if (bonemeal)
        {
            if (random->nextBoolean())
            {
                delete feature;
                feature = new MegaPineTreeFeature(false, random->nextBoolean());
                mega = true;
            }
        }
        else
        {
            for (dx = 0; dx >= -1; --dx)
            {
                for (dz = 0; dz >= -1; --dz)
                {
                    if (isTwoByTwoSaplingOfType(level, pos, dx, dz, PlanksBlock::Variant::SPRUCE))
                    {
                        delete feature;
                        feature = new MegaPineTreeFeature(false, random->nextBoolean());
                        mega = true;
                        break;
                    }
                }
                if (mega)
                    break;
            }
        }
        if (!mega)
        {
            dx = 0;
            dz = 0;
            delete feature;
            feature = new SpruceFeature(true);
        }
        break;
    case PlanksBlock::Variant::eDataBirch:
        delete feature;
        feature = new BirchFeature(true, false);
        break;
    case PlanksBlock::Variant::eDataJungle:
    {
        const BlockState* logState = Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT,
                                                                                PlanksBlock::Variant::JUNGLE);
        const BlockState* leavesState = Blocks::leaves->defaultBlockState()
                                            ->setValue(OldLeafBlock::VARIANT, PlanksBlock::Variant::JUNGLE)
                                            ->setValue(LeafBlock::CHECK_DECAY, false);
        if (bonemeal)
        {
            if (random->nextBoolean())
            {
                delete feature;
                feature = new MegaJungleTreeFeature(true, 10, 20, logState, leavesState);
                mega = true;
            }
        }
        else
        {
            for (dx = 0; dx >= -1; --dx)
            {
                for (dz = 0; dz >= -1; --dz)
                {
                    if (isTwoByTwoSaplingOfType(level, pos, dx, dz, PlanksBlock::Variant::JUNGLE))
                    {
                        delete feature;
                        feature = new MegaJungleTreeFeature(true, 10, 20, logState, leavesState);
                        mega = true;
                        break;
                    }
                }
                if (mega)
                    break;
            }
        }
        if (!mega)
        {
            dx = 0;
            dz = 0;
            delete feature;
            feature = new TreeFeature(true, random->nextInt(7) + 4, logState, leavesState, false);
        }
        break;
    }
    case PlanksBlock::Variant::eDataAcacia:
        delete feature;
        feature = new SavannaTreeFeature(true);
        break;
    case PlanksBlock::Variant::eDataDarkOak:
        if (bonemeal)
        {
            delete feature;
            feature = new RoofTreeFeature(true);
            mega = true;
        }
        else
        {
            for (dx = 0; dx >= -1; --dx)
            {
                for (dz = 0; dz >= -1; --dz)
                {
                    if (isSapling(level, pos.offset(dx, 0, dz), PlanksBlock::Variant::DARK_OAK)
                        && isSapling(level, pos.offset(dx + 1, 0, dz), PlanksBlock::Variant::DARK_OAK)
                        && isSapling(level, pos.offset(dx, 0, dz + 1), PlanksBlock::Variant::DARK_OAK)
                        && isSapling(level, pos.offset(dx + 1, 0, dz + 1), PlanksBlock::Variant::DARK_OAK))
                    {
                        delete feature;
                        feature = new RoofTreeFeature(true);
                        mega = true;
                        break;
                    }
                }
                if (mega)
                    break;
            }
            if (!mega)
            {
                delete feature;
                return;
            }
        }
        break;
    }

    const BlockState* air = Blocks::air->defaultBlockState();
    if (mega)
    {
        level->setBlock(pos.offset(dx, 0, dz), air, Block::UPDATE_INVISIBLE, false);
        level->setBlock(pos.offset(dx + 1, 0, dz), air, Block::UPDATE_INVISIBLE, false);
        level->setBlock(pos.offset(dx, 0, dz + 1), air, Block::UPDATE_INVISIBLE, false);
        level->setBlock(pos.offset(dx + 1, 0, dz + 1), air, Block::UPDATE_INVISIBLE, false);
    }
    else
    {
        level->setBlock(pos, air, Block::UPDATE_INVISIBLE, false);
    }

    bool placed = feature->place(level, *random, pos.offset(dx, 0, dz));
    if (!placed)
    {
        if (mega)
        {
            level->setBlock(pos.offset(dx, 0, dz), state, Block::UPDATE_INVISIBLE, false);
            level->setBlock(pos.offset(dx + 1, 0, dz), state, Block::UPDATE_INVISIBLE, false);
            level->setBlock(pos.offset(dx, 0, dz + 1), state, Block::UPDATE_INVISIBLE, false);
            level->setBlock(pos.offset(dx + 1, 0, dz + 1), state, Block::UPDATE_INVISIBLE, false);
        }
        else
        {
            level->setBlock(pos, state, Block::UPDATE_INVISIBLE, false);
        }
    }
    delete feature;
}

void SaplingBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    if (((GameMode*)Minecraft::GetInstance()->mGameMode)->isCreative())
        growTree(level, pos, state, random, false);
    else
        advanceTree(level, pos, state, random);
}

void SaplingBlock::blockStaticCtor()
{
    std::wstring name(L"type");
    EnumProperty<PlanksBlock::Variant*>* type;
    const Predicate<PlanksBlock::Variant*>* filter = Predicates<PlanksBlock::Variant*>::alwaysTrue;
    {
        boost::unordered_set<PlanksBlock::Variant*> values;
        for (int i = 0; i < 6; ++i)
            values.insert(PlanksBlock::Variant::BY_DATA[i]);
        type = EnumProperty<PlanksBlock::Variant*>::create(name, typeid(PlanksBlock::Variant), values, filter);
    }
    TYPE = type;
}

std::wstring SaplingBlock::getName()
{
    return std::wstring(L10N::GetString(IDS_TILE_SAPLING));
}
const AABB* SaplingBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return SAPLING_SHAPE;
}

void SaplingBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!level->mIsClientSide)
    {
        Bush::tick(level, pos, state, random);
        if (level->getRawBrightness(pos.above()) >= 9 && random->nextInt(7) == 0)
            advanceTree(level, pos, state, random);
    }
}

void SaplingBlock::advanceTree(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (state->getValue(gSaplingStage) == 0)
        level->setBlock(pos, state->cycle(gSaplingStage), Block::UPDATE_INVISIBLE, false);
    else
        growTree(level, pos, state, random, false);
}

int SaplingBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

const BlockState* SaplingBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(TYPE, PlanksBlock::Variant::byData(data & 7))
        ->setValue(gSaplingStage, (data >> 3) & 1);
}

int SaplingBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(TYPE)->getData();
    return data | (state->getValue(gSaplingStage) << 3);
}

unsigned int SaplingBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        return IDS_TOOLTIPS_GROW;
    return Block::GetInteractTooltip(tooltipData);
}

bool SaplingBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* SaplingBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {TYPE, gSaplingStage};
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* SaplingBlock::getTexture(const Direction*, const BlockState* state)
{
    return mIcons[state->getValue(TYPE)->getData()];
}

void SaplingBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 6; i++)
    {
        mIcons[i] = iconRegister->registerIcon(SAPLING_ICON_NAMES[i]);
    }
}

int SaplingBlock::getDescriptionId(int auxValue)
{
    const BlockState* state = getBlockState(auxValue);
    int data = state->getValue(TYPE)->getData();
    if (data < 0)
        data = 0;
    return SAPLING_DESCRIPTION_IDS[data];
}
