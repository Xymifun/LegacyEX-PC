#include "game/world/generation/AbstractClientPlayer.hpp"

#include "game/core/Abilities.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

AbstractClientPlayer::AbstractClientPlayer(Level* level, const std::wstring& name)
    : Player(level, name),
      mElytraRotX(15.0f * Mth::DEG_TO_RAD),
      mElytraRotY(0.0f),
      mElytraRotZ(-15.0f * Mth::DEG_TO_RAD)
{
    mField0x738 = new ItemCooldowns();
}

void AbstractClientPlayer::tick()
{
    Player::tick();
}

bool AbstractClientPlayer::isPickable()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    bool allowPvP = true;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        allowPvP = miniGame->AllowPvP();
    }

    return allowPvP && Player::isPickable();
}

bool AbstractClientPlayer::isSpectator()
{
    return GetGameType() == GameType::SPECTATOR;
}

bool AbstractClientPlayer::isCreative()
{
    return GetGameType() == GameType::CREATIVE;
}

float AbstractClientPlayer::getFieldOfViewModifier()
{
    float modifier = 1.0f;
    if (mAbilities.mFlying)
    {
        modifier = 1.1f;
    }

    AttributeInstance* movementSpeed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    modifier *= (float)(movementSpeed->getValue() / mAbilities.getWalkingSpeed() + 1.0) * 0.5f;

    if (mAbilities.getWalkingSpeed() == 0.0f)
    {
        modifier = 1.0f;
    }

    if (isUsingItem())
    {
        Item* item = getUseItem().get()->getItem();

        if (item == Items::bow || item == Items::arrow || item == Items::tippedArrow)
        {
            float draw = (float)getTicksUsingItem() / (float)BowItem::GetMaxBowDuration();
            if (draw > 1.0f)
            {
                draw = 1.0f;
            }
            else
            {
                draw *= draw;
            }

            modifier *= 1.0f - draw * 0.15f;
        }
    }

    return modifier;
}
