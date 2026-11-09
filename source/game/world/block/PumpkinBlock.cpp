#include "game/world/block/PumpkinBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/SnowMan.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockMaterialPredicate.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPatternBuilder.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/item/ItemId.hpp"

bool pumpkinsPredicate_apply(const BlockState* state)
{
    return state != NULL
           && (state->getBlock() == Blocks::pumpkin || state->getBlock() == Blocks::lit_pumpkin
               || Blocks::carved_pumpkin != NULL);
}

static Predicate<const BlockState*>* sPumpkinsPredicate = new Predicates<const BlockState*>::FunctionPredicate(
    pumpkinsPredicate_apply);

PumpkinBlock::PumpkinBlock() : HorizontalDirectionalBlock(Material::VEGETABLE, MaterialColor::COLOR_ORANGE)
{
    DerivedInit();
    mTopIcon = NULL;
    mFaceIcon = NULL;
    mSnowGolemBasePattern = NULL;
    mSnowGolemFullPattern = NULL;
    mIronGolemBasePattern = NULL;
    mIronGolemFullPattern = NULL;
    registerDefaultState(
        mStateDefinition->any()->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, Direction::SOUTH));
    setTicking(true);
}

BlockPattern* PumpkinBlock::getOrCreateSnowGolemFull()
{
    if (mSnowGolemFullPattern == NULL)
    {
        static std::wstring rows[3] = {L"^", L"#", L"#"};
        mSnowGolemFullPattern = BlockPatternBuilder::start()
                                    ->aisle(rows)
                                    ->where(L'^', BlockInWorld::hasState(sPumpkinsPredicate))
                                    ->where(L'#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::snow)))
                                    ->build(NULL, NULL);
    }
    return mSnowGolemFullPattern;
}

void PumpkinBlock::fjAwardCraftedGolemStat(unsigned long long craftedEntity)
{
    boost::shared_ptr<Player> player = BlockItem::fjGetCurrentlyPlacingBlockPlayer();
    if (player)
        player->awardStat(GenericStats::craftedEntity(craftedEntity), GenericStats::param_craftedEntity(craftedEntity));
}

BlockPattern* PumpkinBlock::getOrCreateIronGolemFull()
{
    if (mIronGolemFullPattern == NULL)
    {
        static std::wstring rows[3] = {L"~^~", L"###", L"~#~"};
        mIronGolemFullPattern =
            BlockPatternBuilder::start()
                ->aisle(rows)
                ->where(L'^', BlockInWorld::hasState(sPumpkinsPredicate))
                ->where(L'#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::iron_block)))
                ->where(L'~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::AIR)))
                ->build(NULL, NULL);
    }
    return mIronGolemFullPattern;
}

BlockPattern* PumpkinBlock::getOrCreateSnowGolemBase()
{
    if (mSnowGolemBasePattern == NULL)
    {
        static std::wstring rows[3] = {L" ", L"#", L"#"};
        mSnowGolemBasePattern = BlockPatternBuilder::start()
                                    ->aisle(rows)
                                    ->where(L'#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::snow)))
                                    ->build(NULL, NULL);
    }
    return mSnowGolemBasePattern;
}

BlockPattern* PumpkinBlock::getOrCreateIronGolemBase()
{
    if (mIronGolemBasePattern == NULL)
    {
        static std::wstring rows[3] = {L"~ ~", L"###", L"~#~"};
        mIronGolemBasePattern =
            BlockPatternBuilder::start()
                ->aisle(rows)
                ->where(L'#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::iron_block)))
                ->where(L'~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::AIR)))
                ->build(NULL, NULL);
    }
    return mIronGolemBasePattern;
}

bool PumpkinBlock::canSpawnGolem(Level* level, const BlockPos& pos)
{
    return getOrCreateSnowGolemBase()->find(level, pos) != NULL || getOrCreateIronGolemBase()->find(level, pos) != NULL;
}

void PumpkinBlock::trySpawnGolem(Level* level, const BlockPos& pos)
{
    BlockPattern::BlockPatternMatch* snow = getOrCreateSnowGolemFull()->find(level, pos);
    if (snow != NULL)
    {
        bool canCreate = level->canCreateMore(EntityType::SNOW_MAN, Level::eSpawnType_Egg);

        for (int y = 0; y < static_cast<int>(getOrCreateSnowGolemFull()->getHeight()); ++y)
        {
            BlockInWorld* blockInWorld = snow->getBlock(0, y, 0);
            if (!canCreate)
            {
                Block* block = snow->getBlock(0, y, 0)->getState()->getBlock();
                block->spawnResources(level, snow->getBlock(0, y, 0)->getPos(),
                                      level->getBlockState(snow->getBlock(0, y, 0)->getPos()), 0);
            }
            level->setBlock(blockInWorld->getPos(), Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }

        BlockPos spawnPos = snow->getBlock(0, 2, 0)->getPos();
        if (canCreate)
        {
            boost::shared_ptr<SnowMan> snowMan(new SnowMan(level));
            snowMan->moveTo((double)spawnPos.x + 0.5, (double)spawnPos.y + 0.05, (double)spawnPos.z + 0.5, 0.0f, 0.0f);
            level->addEntity(snowMan);
            fjAwardCraftedGolemStat(EntityType::SNOW_MAN);
        }

        for (int i = 0; i < 120; ++i)
        {
            level->addParticle(PARTICLE_SNOW_SHOVEL, (double)spawnPos.x + level->mRandom->nextDouble(),
                               (double)spawnPos.y + level->mRandom->nextDouble() * 2.5,
                               (double)spawnPos.z + level->mRandom->nextDouble(), 0.0, 0.0, 0.0,
                               arrayWithLength<int>());
        }

        for (int y = 0; y < static_cast<int>(getOrCreateSnowGolemFull()->getHeight()); ++y)
            level->blockUpdated(snow->getBlock(0, y, 0)->getPos(), Blocks::air, false);
        return;
    }

    BlockPattern::BlockPatternMatch* iron = getOrCreateIronGolemFull()->find(level, pos);
    if (iron != NULL)
    {
        bool canCreate = level->canCreateMore(EntityType::VILLAGER_GOLEM, Level::eSpawnType_Egg);

        for (int x = 0; x < static_cast<int>(getOrCreateIronGolemFull()->getWidth()); ++x)
        {
            for (int y = 0; y < static_cast<int>(getOrCreateIronGolemFull()->getHeight()); ++y)
            {
                if (!canCreate)
                {
                    Block* block = iron->getBlock(x, y, 0)->getState()->getBlock();
                    block->spawnResources(level, iron->getBlock(x, y, 0)->getPos(),
                                          level->getBlockState(iron->getBlock(x, y, 0)->getPos()), 0);
                }
                level->setBlock(iron->getBlock(x, y, 0)->getPos(), Blocks::air->defaultBlockState(),
                                Block::UPDATE_CLIENTS, false);
            }
        }

        BlockPos spawnPos = iron->getBlock(1, 2, 0)->getPos();
        if (canCreate)
        {
            boost::shared_ptr<VillagerGolem> villagerGolem(new VillagerGolem(level));
            villagerGolem->setPlayerCreated(true);
            villagerGolem->moveTo((double)spawnPos.x + 0.5, (double)spawnPos.y + 0.05, (double)spawnPos.z + 0.5, 0.0f,
                                  0.0f);
            level->addEntity(villagerGolem);
            fjAwardCraftedGolemStat(EntityType::VILLAGER_GOLEM);
        }

        for (int i = 0; i < 120; ++i)
        {
            level->addParticle(PARTICLE_SNOWBALL_POOF, (double)spawnPos.x + level->mRandom->nextDouble(),
                               (double)spawnPos.y + level->mRandom->nextDouble() * 3.9,
                               (double)spawnPos.z + level->mRandom->nextDouble(), 0.0, 0.0, 0.0,
                               arrayWithLength<int>());
        }

        for (int x = 0; x < static_cast<int>(getOrCreateIronGolemFull()->getWidth()); ++x)
        {
            for (int y = 0; y < static_cast<int>(getOrCreateIronGolemFull()->getHeight()); ++y)
                level->blockUpdated(iron->getBlock(x, y, 0)->getPos(), Blocks::air, false);
        }
    }
}

void PumpkinBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    trySpawnGolem(level, pos);
}

bool PumpkinBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const BlockState* below = level->getBlockState(pos.below());
    return (level->getBlockState(pos)->getBlock()->mMaterial->isReplaceable()
            || level->getBlockState(pos)->getBlock() == Blocks::air)
           && below->canProvideSupport(Direction::UP, BLOCK_SUPPORT_ANY);
}

const BlockState* PumpkinBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(
        HorizontalDirectionalBlock::FACING_PROPERTY,
        rotation->rotate(state->getTypedBoxed(HorizontalDirectionalBlock::FACING_PROPERTY)->getValue()));
}

const BlockState* PumpkinBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = HorizontalDirectionalBlock::FACING_PROPERTY;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

const BlockState* PumpkinBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float,
                                                     int, boost::shared_ptr<LivingEntity> placer)
{
    return defaultBlockState()->setValue(HorizontalDirectionalBlock::FACING_PROPERTY,
                                         placer->getDirection()->getOpposite());
}

const BlockState* PumpkinBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, Direction::from2DDataValue(data));
}

int PumpkinBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue();
}

BlockStateDefinition* PumpkinBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HorizontalDirectionalBlock::FACING_PROPERTY};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* PumpkinBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    bool isFaceBlock = this == Blocks::lit_pumpkin || this == Blocks::carved_pumpkin;
    if (direction == Direction::UP || direction == Direction::DOWN)
        return mTopIcon;
    if (facing == direction && isFaceBlock)
        return mFaceIcon;
    return mIcon;
}

void PumpkinBlock::registerIcons(IconRegister* iconRegister)
{
    bool isLit = this == Blocks::lit_pumpkin;
    mFaceIcon = iconRegister->registerIcon(getIconName() + L"_face_" + (isLit ? L"on" : L"off"));
    mTopIcon = iconRegister->registerIcon(getIconName() + L"_top");
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
}

unsigned int PumpkinBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (this == Blocks::pumpkin && Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::SHEARS
        && tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success)
        return IDS_TOOLTIPS_CARVE;
    return Block::GetInteractTooltip(tooltipData);
}
