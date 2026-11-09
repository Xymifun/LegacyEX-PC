#include "game/item/SaddleItem.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Pig.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"

SaddleItem::SaddleItem() : Item()
{
    mMaxStackSize = 1;
}

bool SaddleItem::interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                               boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand)
{
    if (target && eTYPE_DERIVED_FROM(EntityType::PIG, target->GetType()))
    {
        boost::shared_ptr<Pig> pig = boost::static_pointer_cast<Pig>(target);
        if (!pig->hasSaddle() && !pig->isBaby())
        {
            pig->setSaddle(true);
            pig->mLevel->playSound(player, pig->mX, pig->mY, pig->mZ, SoundEvents::PIG_SADDLE,
                                   SoundSource::eSoundSource_Neutral, 0.5f, 1.0f, 16.0f);
            itemInstance->shrink(1);
        }
        return true;
    }
    return false;
}
