#include "game/item/FoodItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

FoodItem::FoodItem(int nutrition, float saturationModifier, bool isMeat)
    : Item(),
      mNutrition(nutrition),
      mSaturationModifier(saturationModifier),
      mIsMeat(isMeat)
{
    _init();
}

FoodItem::FoodItem(int nutrition, bool isMeat)
    : Item(),
      mNutrition(nutrition),
      mSaturationModifier(0.6f),
      mIsMeat(isMeat)
{
    _init();
}

void FoodItem::_init()
{
    mIsFastFood = false;
    mCanAlwaysEat = false;
    mEffectProbability = 0.0f;
    mEatEffect = NULL;
}

not_null_ptr<ItemInstance> FoodItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                     boost::shared_ptr<LivingEntity> entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        player->awardStat(GenericStats::itemsUsed(itemInstance->getItem()),
                          GenericStats::param_itemsUsed(player, itemInstance));
        if (itemInstance->getItem() == Items::rottenFlesh && player->getFoodData()->getFoodLevel() == 0)
        {
            player->awardStat(GenericStats::ironBelly(), GenericStats::param_ironBelly());
        }
        player->SetLastAteKelp(itemInstance->getItem() == Items::driedKelp);
    }

    if (!level->mIsClientSide)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            player->getFoodData()->eat(this, itemInstance);
            level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ, SoundEvents::PLAYER_BURP,
                             SoundSource::eSoundSource_Players, 0.5f, level->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
            addEatEffect(itemInstance, level, player);
        }

        bool consume = true;
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            if (player->mAbilities.mInstabuild)
            {
                consume = false;
            }
        }
        if (consume)
        {
            itemInstance->shrink(1);
        }
    }

    return itemInstance;
}

void FoodItem::addEatEffect(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player)
{
    if (!level->mIsClientSide && mEatEffect != NULL && level->mRandom->nextFloat() < mEffectProbability)
    {
        player->addEffect(new MobEffectInstance(mEatEffect), boost::shared_ptr<Entity>());
    }
}

int FoodItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return mIsFastFood ? 16 : 32;
}

int FoodItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return UseAnim_eat;
}

InteractionResultHolder FoodItem::use(Level* level, boost::shared_ptr<Player> player,
                                      InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (player->canEat(mCanAlwaysEat))
    {
        player->startUsingItem(hand);
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
    }
    return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
}

bool FoodItem::canEat(boost::shared_ptr<Player> player)
{
    return player->canEat(mCanAlwaysEat);
}

unsigned int FoodItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    const bool canUse = canEat(tooltipData.m_pPlayer);
    if (canUse)
    {
        return IDS_TOOLTIPS_EAT;
    }
    return IDS_NONE;
}

int FoodItem::getNutrition(not_null_ptr<ItemInstance> itemInstance)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        return miniGame->getNutrition(Item::VALIDgetId(itemInstance->getItem()), mNutrition);
    }
    return mNutrition;
}

float FoodItem::getSaturationModifier(not_null_ptr<ItemInstance> itemInstance)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        return miniGame->getSaturationModifier(Item::VALIDgetId(itemInstance->getItem()), mSaturationModifier);
    }
    return mSaturationModifier;
}

bool FoodItem::isMeat()
{
    return mIsMeat;
}

FoodItem* FoodItem::setEatEffect(MobEffectInstance* effect, float probability)
{
    delete mEatEffect;
    mEatEffect = effect;
    mEffectProbability = probability;
    return this;
}

FoodItem* FoodItem::setCanAlwaysEat()
{
    mCanAlwaysEat = true;
    return this;
}

void FoodItem::setIsFastFood()
{
    mIsFastFood = true;
}
