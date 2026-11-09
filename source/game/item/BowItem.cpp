#include "game/item/BowItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <string>
#include <string.h>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/entity/projectile/ArrowItem.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BowItem_Pull_ItemPropertyFunction.hpp"
#include "game/world/block/BowItem_Pulling_ItemPropertyFunction.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

static const std::wstring BOW_PULL_ICON_NAMES[3] = {std::wstring(L"bow_pull_0"), std::wstring(L"bow_pull_1"),
                                                    std::wstring(L"bow_pull_2")};

BowItem::BowItem()
{
    mMaxStackSize = 1;
    setMaxDamage(384);

    memset(mDrawnIcons, 0, sizeof(mDrawnIcons));

    addProperty(ResourceLocation(std::wstring(L"pull")), new BowItem_Pull_ItemPropertyFunction());
    addProperty(ResourceLocation(std::wstring(L"pulling")), new BowItem_Pulling_ItemPropertyFunction());
}

bool BowItem::isValidProjectile(not_null_ptr<ItemInstance> itemInstance)
{
    return !itemInstance->isEmpty() && dynamic_cast<ArrowItem*>(itemInstance->getItem()) != NULL;
}

not_null_ptr<ItemInstance> BowItem::findProjectile(boost::shared_ptr<Player> player)
{
    if (isValidProjectile(player->getItemInHand(InteractionHand::eInteractionHand_OffHand)))
    {
        return player->getItemInHand(InteractionHand::eInteractionHand_OffHand);
    }

    if (isValidProjectile(player->getItemInHand(InteractionHand::eInteractionHand_MainHand)))
    {
        return player->getItemInHand(InteractionHand::eInteractionHand_MainHand);
    }

    for (unsigned int slot = 0; slot < player->mInventory->getContainerSize(); slot++)
    {
        not_null_ptr<ItemInstance> itemInstance = player->mInventory->getItem(slot);

        if (isValidProjectile(itemInstance))
        {
            return itemInstance;
        }
    }

    return ItemInstance::EMPTY;
}

int BowItem::GetMaxBowDuration()
{
    int maxDuration = 20;

    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureArrowCanKnockback())
    {
        maxDuration = 10;
    }

    return maxDuration;
}

float BowItem::getPowerForTime(int charge)
{
    float power = (float)charge / (float)GetMaxBowDuration();

    power = (power * power + (power + power)) / 3.0f;

    if (power > 1.0f)
    {
        power = 1.0f;
    }

    return power;
}

void BowItem::fjReleaseUsingWithProjectile(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                           boost::shared_ptr<LivingEntity> entity, int timeLeft,
                                           not_null_ptr<ItemInstance> projectile)
{
    bool isPlayer = (entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER;

    if (!isPlayer)
    {
        return;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);

    bool infiniteAmmo = player->mAbilities.mInstabuild
                        || EnchantmentHelper::getItemEnchantmentLevel(Enchantments::INFINITY_ARROWS, itemInstance) > 0;

    if (projectile->isEmpty() && !infiniteAmmo)
    {
        return;
    }

    if (projectile->isEmpty())
    {
        projectile = not_null_ptr<ItemInstance>(new ItemInstance(Items::arrow));
    }

    float power = getPowerForTime(getUseDuration(itemInstance) - timeLeft);

    if ((double)power < 0.1)
    {
        return;
    }

    bool creativeArrow = infiniteAmmo && projectile->getItem() == Items::arrow;

    if (!level->mIsClientSide)
    {
        ArrowItem* arrowItem = dynamic_cast<ArrowItem*>(projectile->getItem()) != NULL
                                   ? static_cast<ArrowItem*>(projectile->getItem())
                                   : static_cast<ArrowItem*>(Items::arrow);

        boost::shared_ptr<Arrow> arrow = arrowItem->createArrow(level, projectile, player);
        arrow->shootFromRotation(player, player->mXRot, player->mYHeadRot, 0.0f, power * 3.0f, 1.0f);

        if (power == 1.0f)
        {
            arrow->setCritArrow(true);
        }

        int powerLevel = EnchantmentHelper::getItemEnchantmentLevel(Enchantments::POWER_ARROWS, itemInstance);

        if (powerLevel > 0)
        {
            arrow->setEnchantPower(powerLevel);
        }

        int punchLevel = EnchantmentHelper::getItemEnchantmentLevel(Enchantments::PUNCH_ARROWS, itemInstance);

        if (punchLevel > 0)
        {
            arrow->setKnockback(punchLevel);
        }

        int flameLevel = EnchantmentHelper::getItemEnchantmentLevel(Enchantments::FLAMING_ARROWS, itemInstance);

        if (flameLevel > 0)
        {
            arrow->setSecondsOnFire(100);
        }

        itemInstance->hurtAndBreak(1, player);

        if (creativeArrow
            || (player->mAbilities.mInstabuild
                && (projectile->getItem() == Items::spectralArrow || projectile->getItem() == Items::tippedArrow)))
        {
            arrow->setPickup(AbstractRotatedItemProjectile::ePickup_CreativeOnly);
        }

        level->addEntity(arrow);
    }

    level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ, SoundEvents::ARROW_SHOOT,
                     SoundSource::eSoundSource_Players, 1.0f, 1.0f / (random->nextFloat() * 0.4f + 1.2f) + power * 0.5f,
                     16.0f);

    if (!creativeArrow && !player->mAbilities.mInstabuild)
    {
        projectile->shrink(1);

        if (projectile->isEmpty())
        {
            player->mInventory->removeItem(projectile);
        }
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));
}

void BowItem::releaseUsing(not_null_ptr<ItemInstance> itemInstance, Level* level,
                           boost::shared_ptr<LivingEntity> entity, int timeLeft)
{
    bool isPlayer = (entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER;

    if (!isPlayer)
    {
        return;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);

    not_null_ptr<ItemInstance> projectile = findProjectile(player);

    fjReleaseUsingWithProjectile(itemInstance, level, player, timeLeft, projectile);
}

int BowItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return 72000;
}

int BowItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return UseAnim_bow;
}

InteractionResultHolder BowItem::fjUseWithProjectile(Level* level, boost::shared_ptr<Player> player,
                                                     InteractionHand::EInteractionHand hand,
                                                     not_null_ptr<ItemInstance> projectile)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    bool hasProjectile = !projectile->isEmpty();

    if (player->mAbilities.mInstabuild || hasProjectile)
    {
        player->startUsingItem(hand);

        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
}

InteractionResultHolder BowItem::use(Level* level, boost::shared_ptr<Player> player,
                                     InteractionHand::EInteractionHand hand)
{
    return fjUseWithProjectile(level, player, hand, findProjectile(player));
}

int BowItem::getEnchantmentValue()
{
    return 1;
}

void BowItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);

    for (int i = 0; i < 3; i++)
    {
        mDrawnIcons[i] = iconRegister->registerIcon(BOW_PULL_ICON_NAMES[i]);
    }
}

TextureAtlasSprite* BowItem::getDrawnIcon(int stage)
{
    return mDrawnIcons[stage];
}

int BowItem::GetOverrideCount()
{
    return 0;
}

int BowItem::GetOverrideCountColour()
{
    return Item::GetOverrideCountColour();
}

TextureAtlasSprite* BowItem::GetOverrideCountIcon(not_null_ptr<ItemInstance> itemInstance)
{
    if (Minecraft::GetInstance()->mPlayer->getUseItem() == itemInstance)
    {
        int ticksUsingItem = Minecraft::GetInstance()->mPlayer->getTicksUsingItem();

        if (ticksUsingItem >= GetMaxBowDuration() - 2)
        {
            return static_cast<BowItem*>(Items::bow)->getDrawnIcon(2);
        }

        if (ticksUsingItem > GetMaxBowDuration() * 2 / 3)
        {
            return static_cast<BowItem*>(Items::bow)->getDrawnIcon(1);
        }

        if (ticksUsingItem > 0)
        {
            return static_cast<BowItem*>(Items::bow)->getDrawnIcon(0);
        }
    }

    return Item::GetOverrideCountIcon(itemInstance);
}

unsigned int BowItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pPlayer->mAbilities.mInstabuild || tooltipData.m_pPlayer->mInventory->hasResource(Items::arrow)
        || tooltipData.m_pPlayer->mInventory->hasResource(Items::tippedArrow))
    {
        return tooltipData.m_pPlayer->isUsingItem() ? IDS_TOOLTIPS_RELEASE_BOW : IDS_TOOLTIPS_DRAW_BOW;
    }

    return IDS_NONE;
}
