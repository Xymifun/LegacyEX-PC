#include "game/item/DyeItem.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cstring>
#include <string>

#include "game/client/render/ParticleType.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Sheep.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BonemealableBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/generation/Player.hpp"

const int DyeItem::DYE_COLORS[16] = {0x1E1B1B, 0xB3312C, 0x3B511A, 0x51301A, 0x253192, 0x7B2FBE, 0x287697, 0xABABAB,
                                     0x434343, 0xD88198, 0x41CD34, 0xDECF2A, 0x6689D3, 0xC354CD, 0xEB8844, 0xF0F0F0};

DyeItem::DyeItem() : Item()
{
    setStackedByData(true);
    setMaxDamage(0);
    memset(mIcons, 0, sizeof(mIcons));
}

int DyeItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return DyeColor::byItemData(itemInstance->getAuxValue())->getName();
}

int DyeItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return DyeColor::byItemData(itemInstance->getAuxValue())->getUseName();
}

bool DyeItem::growCrop(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockPos& pos, bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();

    if (block != NULL)
    {
        BonemealableBlock* crop = dynamic_cast<BonemealableBlock*>(block);

        if (crop != NULL && crop->isValidBonemealTarget(level, pos, state, level->mIsClientSide)
            && crop->isBonemealSuccess(level, level->mRandom, pos, state))
        {
            if (!simulate)
            {
                if (!level->mIsClientSide)
                {
                    crop->performBonemeal(level, level->mRandom, pos, state);
                }

                itemInstance->shrink(1);
            }

            return true;
        }
    }

    return false;
}

InteractionResult::EInteractionResult DyeItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                     const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                     const Direction* direction, float hitX, float hitY, float hitZ,
                                                     bool simulate)
{
    BlockPos blockPos(pos);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(blockPos.relative(direction), direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    const DyeColor* color = DyeColor::byItemData(itemInstance->getAuxValue());

    if (color == DyeColor::WHITE)
    {
        if (growCrop(itemInstance, level, blockPos, simulate))
        {
            if (!level->mIsClientSide)
            {
                level->levelEvent(LevelEvent::PARTICLES_PLANT_GROWTH, blockPos, 0);
            }

            return InteractionResult::eInteractionResult_Success;
        }
    }
    else if (color == DyeColor::BROWN)
    {
        const BlockState* state = level->getBlockState(blockPos);
        Block* block = state->getBlock();

        if (block == Blocks::log && state->getValue(OldLogBlock::VARIANT) == PlanksBlock::Variant::JUNGLE)
        {
            if (direction == Direction::DOWN || direction == Direction::UP)
            {
                return InteractionResult::eInteractionResult_Fail;
            }

            blockPos = blockPos.relative(direction);

            if (level->isEmptyBlock(blockPos))
            {
                if (!simulate)
                {
                    const BlockState* cocoa = Blocks::cocoa->getStateForPlacement(level, blockPos, direction, hitX,
                                                                                  hitY, hitZ, 0, player);
                    level->setBlock(blockPos, cocoa, Block::UPDATE_CLIENTS | Block::UPDATE_IMMEDIATE, false);

                    if (!player->mAbilities.mInstabuild)
                    {
                        itemInstance->shrink(1);
                    }
                }

                return InteractionResult::eInteractionResult_Success;
            }
        }

        return InteractionResult::eInteractionResult_Fail;
    }

    return InteractionResult::eInteractionResult_Pass;
}

void DyeItem::addGrowthParticles(Level* level, const BlockPos& pos, int count)
{
    if (count == 0)
    {
        count = 15;
    }

    const BlockState* state = level->getBlockState(pos);

    if (state->getMaterial() != Material::AIR)
    {
        for (int i = 0; i < count; ++i)
        {
            double xa = level->mRandom->nextGaussian() * 0.02;
            double ya = level->mRandom->nextGaussian() * 0.02;
            double za = level->mRandom->nextGaussian() * 0.02;
            level->addParticle(PARTICLE_HAPPY_VILLAGER, (double)((float)pos.x + level->mRandom->nextFloat()),
                               (double)pos.y + (double)level->mRandom->nextFloat() * state->getShape(level, pos)->mMaxY,
                               (double)((float)pos.z + level->mRandom->nextFloat()), xa, ya, za,
                               arrayWithLength<int>());
        }
    }
}

bool DyeItem::interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                            boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand)
{
    bool isSheep = eTYPE_DERIVED_FROM(EntityType::SHEEP, target->GetType());

    if (isSheep)
    {
        boost::shared_ptr<Sheep> sheep = boost::static_pointer_cast<Sheep>(target);
        const DyeColor* color = DyeColor::byItemData(itemInstance->getAuxValue());

        if (!sheep->isSheared() && sheep->getColor() != color)
        {
            sheep->setColor(color);
            itemInstance->shrink(1);
        }

        return true;
    }

    return false;
}

TextureAtlasSprite* DyeItem::getIcon(int auxValue)
{
    return mIcons[Mth::clamp(auxValue, 0, 15)];
}

void DyeItem::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 16; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(getIconName() + L"_" + DyeColor::TEXTURE_NAMES[i]);
    }
}
