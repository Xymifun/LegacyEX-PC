#include "game/item/ChorusFruitItem.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

ChorusFruitItem::ChorusFruitItem(int nutrition, float saturationModifier)
    : FoodItem(nutrition, saturationModifier, false)
{
}

not_null_ptr<ItemInstance> ChorusFruitItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                            boost::shared_ptr<LivingEntity> entity)
{
    not_null_ptr<ItemInstance> result = FoodItem::finishUsingItem(itemInstance, level, entity);

    if (!level->mIsClientSide)
    {
        double oldX = entity->mX;
        double oldY = entity->mY;
        double oldZ = entity->mZ;

        for (int attempt = 0; attempt < 16; ++attempt)
        {
            double x = entity->mX + (entity->getRandom()->nextDouble() - 0.5) * 16.0;
            double y = Mth::clamp(entity->mY + (double)(entity->getRandom()->nextInt(16) - 8), 0.0,
                                  (double)level->getHeight() - 1.0);
            double z = entity->mZ + (entity->getRandom()->nextDouble() - 0.5) * 16.0;

            if (entity->isPassenger())
            {
                entity->stopRidingBeforeTeleport();
            }

            if (entity->randomTeleport(x, y, z))
            {
                level->playSound(boost::shared_ptr<Player>(), oldX, oldY, oldZ, SoundEvents::CHORUS_FRUIT_TELEPORT,
                                 SoundSource::eSoundSource_Players, 1.0f, 1.0f, 16.0f);
                entity->playSound(SoundEvents::CHORUS_FRUIT_TELEPORT, 1.0f, 1.0f);
                break;
            }
        }

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            player->getCooldowns()->addCooldown(this, 20);
        }
    }

    return result;
}
