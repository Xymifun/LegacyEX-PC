#include "game/world/block/TntBlock.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Items.hpp"

AbstractProperty<bool>* TntBlock::EXPLODE = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"explode"));

TntBlock::TntBlock() : Block(Material::EXPLOSIVE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(EXPLODE, false));
}

void TntBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    if (level->hasNeighborSignal(pos) && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        destroy(level, pos, state->setValue(EXPLODE, true));
        level->removeBlock(pos);
    }
}

void TntBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (level->hasNeighborSignal(pos) && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        destroy(level, pos, state->setValue(EXPLODE, true));
        level->removeBlock(pos);
    }
}

void TntBlock::wasExploded(Level* level, const BlockPos& pos, Explosion* explosion)
{
    if (level->mIsClientSide || !level->newPrimedTntAllowed() || !gApp.GetGameHostOption(eGameHostOption_TNT))
        return;

    boost::shared_ptr<PrimedTnt> primedTnt(
        new PrimedTnt(level, pos.x + 0.5f, pos.y, pos.z + 0.5f, explosion->getSourceMob()));
    primedTnt->setFuse(level->mRandom->nextInt(primedTnt->getLife() / 4) + primedTnt->getLife() / 8);
    level->addEntity(primedTnt);
}

void TntBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
    destroy(level, pos, state, boost::shared_ptr<LivingEntity>());
}

void TntBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state,
                       boost::shared_ptr<LivingEntity> entity)
{
    if (level->mIsClientSide)
        return;

    if ((state->getValue(EXPLODE) || Minecraft::InMiniGame(eMiniGameId_Tumble, true)) && level->newPrimedTntAllowed()
        && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        boost::shared_ptr<PrimedTnt> primedTnt(new PrimedTnt(level, pos.x + 0.5f, pos.y, pos.z + 0.5f, entity));
        level->addEntity(primedTnt);
        level->playSound(boost::shared_ptr<Player>(), primedTnt->mX, primedTnt->mY, primedTnt->mZ,
                         SoundEvents::TNT_PRIMED, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
    }
}

bool TntBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                   InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                   float hitZ, bool clientSide)
{
    if (clientSide)
        return false;

    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!item->isEmpty())
    {
        Item* held = item->getItem();
        if (held == Items::flintAndSteel || held == Items::fireCharge)
        {
            destroy(level, pos, state->setValue(EXPLODE, true), player);
            level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            if (held == Items::flintAndSteel)
                item->hurtAndBreak(1, player);
            else if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            return true;
        }
    }
    return Block::use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, false);
}

void TntBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                            const boost::shared_ptr<Entity>& entity)
{
    if (Minecraft::InMiniGame(eMiniGameId_Battle, true) || level->mIsClientSide)
        return;

    if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType()) && entity->isOnFire())
    {
        boost::shared_ptr<Arrow> arrow = boost::dynamic_pointer_cast<Arrow>(entity);
        if (arrow->mOwner == NULL)
        {
            destroy(level, pos, level->getBlockState(pos)->setValue(EXPLODE, true), boost::shared_ptr<LivingEntity>());
        }
        else
        {
            const BlockState* primedState = level->getBlockState(pos)->setValue(EXPLODE, true);
            destroy(level, pos, primedState,
                    eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, arrow->mOwner->GetType())
                        ? boost::dynamic_pointer_cast<LivingEntity>(arrow->mOwner)
                        : boost::shared_ptr<LivingEntity>());
        }
        level->removeBlock(pos);
    }
}

bool TntBlock::dropFromExplosion(Explosion*)
{
    return false;
}

void TntBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                           boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && gApp.GetGameHostOption(eGameHostOption_TNT))
    {
        destroy(level, pos, state->setValue(EXPLODE, true), placer);
        level->removeBlock(pos);
    }
}

const BlockState* TntBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(EXPLODE, (data & 1) > 0);
}

int TntBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(EXPLODE);
}

BlockStateDefinition* TntBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {EXPLODE};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* TntBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::DOWN)
        return mBottomIcon;
    if (direction == Direction::UP)
        return mTopIcon;
    return mIcon;
}

void TntBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"tnt_side");
    mTopIcon = iconRegister->registerIcon(L"tnt_top");
    mBottomIcon = iconRegister->registerIcon(L"tnt_bottom");
}
