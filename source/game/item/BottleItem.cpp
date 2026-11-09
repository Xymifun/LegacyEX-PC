#include "game/item/BottleItem.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/item/AreaEffectCloud_Predicate.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

BottleItem::BottleItem() : Item()
{
}

not_null_ptr<ItemInstance> BottleItem::turnBottleIntoItem(not_null_ptr<ItemInstance> itemInstance,
                                                          boost::shared_ptr<Player> player,
                                                          not_null_ptr<ItemInstance> result)
{
    itemInstance->shrink(1);
    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));

    if (itemInstance->isEmpty())
        return result;

    if (!player->mInventory->add(result))
    {
        player->drop(result, false);
    }
    return itemInstance;
}

InteractionResultHolder BottleItem::use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand)
{
    AreaEffectCloud_Predicate predicate;
    AABB* area = player->mBoundingBox->grow(2.0);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(AreaEffectCloud), area,
                                                                                  &predicate, false);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!entities->empty())
    {
        boost::shared_ptr<AreaEffectCloud> cloud = boost::static_pointer_cast<AreaEffectCloud>(entities->at(0));
        cloud->setRadius(cloud->getRadius() - 0.5f);
        delete entities;
        player->awardStat(GenericStats::youNeedAMint(), GenericStats::param_youNeedAMint());
        level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ,
                         SoundEvents::BOTTLE_FILL_DRAGONBREATH, SoundSource::eSoundSource_Neutral, 1.0f, 1.0f, 16.0f);
        not_null_ptr<ItemInstance> result(new ItemInstance(Items::dragonBreath));
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success,
                                       turnBottleIntoItem(itemInstance, player, result));
    }

    delete entities;

    HitResult* hit = getPlayerPOVHitResult(level, player, true);
    if (hit != NULL)
    {
        BlockPos constraintPos(hit->mLocation);
        if (!level->mDimension->inPlayerConstraints(constraintPos))
        {
            delete hit;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
        }
        if (hit->mType == HitResult::BLOCK)
        {
            BlockPos pos = hit->getBlockPos();
            if (!level->mayInteract(player, pos, NULL)
                || !player->mayUseItemAt(pos.relative(hit->mDirection), hit->mDirection, itemInstance))
            {
                delete hit;
                return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
            }
            if (level->getBlockState(pos)->getMaterial() == Material::WATER)
            {
                level->playSound(player, player->mX, player->mY, player->mZ,
                                 SoundEvents::BOTTLE_FILL, SoundSource::eSoundSource_Neutral, 1.0f, 1.0f, 16.0f);
                delete hit;
                not_null_ptr<ItemInstance> result(new ItemInstance(Items::potion));
                PotionUtils::setPotion(result, Potions::WATER);
                return InteractionResultHolder(InteractionResult::eInteractionResult_Success,
                                               turnBottleIntoItem(itemInstance, player, result));
            }
        }
        delete hit;
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
}

bool BottleItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    AreaEffectCloud_Predicate predicate;
    AABB* area = player->mBoundingBox->grow(2.0);
    if (level->hasEntitiesOfClass(typeid(AreaEffectCloud), area, &predicate, false))
    {
        return true;
    }

    not_null_ptr<ItemInstance> itemInHand = player->getItemInHand(hand);

    HitResult* hit = getPlayerPOVHitResult(level, player, true);
    if (hit == NULL || !level->mDimension->inPlayerConstraints(BlockPos(hit->mLocation)))
        return false;

    if (hit->mType == HitResult::BLOCK)
    {
        BlockPos pos = hit->getBlockPos();
        if (!level->mayInteract(player, pos, NULL))
        {
            delete hit;
            return false;
        }
        if (!player->mayUseItemAt(pos, hit->mDirection, itemInHand))
        {
            delete hit;
            return false;
        }
        if (level->getBlockState(pos)->getBlock()->mMaterial == Material::WATER)
        {
            delete hit;
            return true;
        }
    }

    delete hit;
    return false;
}

unsigned int BottleItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (TestUse(tooltipData.m_pLevel, tooltipData.m_pPlayer, InteractionHand::eInteractionHand_MainHand))
    {
        return IDS_TOOLTIPS_COLLECT;
    }
    return IDS_NONE;
}

TextureAtlasSprite* BottleItem::getIcon(int)
{
    return Items::potion->getIcon(0);
}

void BottleItem::registerIcons(IconRegister*)
{
}
