#include "game/item/PowerupItems.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/GameMode.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundPowerupPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"

void PowerupItems::DoLargeBoost(boost::shared_ptr<Player> player)
{
    if (player->mLevel->mIsClientSide)
    {
        const double horizontalSpeed = Mth::sqrt(player->mXd * player->mXd + player->mZd * player->mZd);
        player->activateElytraSpeedBoost(horizontalSpeed + 0.6);
    }
}

void PowerupItems::DoSmallBoost(boost::shared_ptr<Player> player)
{
    if (player->mLevel->mIsClientSide)
    {
        const double horizontalSpeed = Mth::sqrt(player->mXd * player->mXd + player->mZd * player->mZd);
        player->activateElytraSpeedBoost(horizontalSpeed + 0.3);
    }
}

void PowerupItems::DoDropTNT(boost::shared_ptr<Player> player)
{
    if (player->mLevel->mIsClientSide)
    {
        return;
    }

    boost::shared_ptr<PrimedTnt> tnt(new PrimedTnt(player->mLevel, player->mX, player->mY, player->mZ, player));
    tnt->setNoGravity(true);
    tnt->setFuse(10);
    player->mLevel->addEntity(tnt);
    player->mLevel->playSound(boost::shared_ptr<Player>(), tnt->mX, tnt->mY, tnt->mZ, SoundEvents::TNT_PRIMED,
                              SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
}

void PowerupItems::DoGainPoint(boost::shared_ptr<Player> player)
{
    player->GetGameMode()->RecordGainPoints(1);
}

bool PowerupItems::IsDebuff(eGlide_Timed_Powerup_ID powerupId)
{
    return powerupId == eGlide_Timed_Powerup_ID_InvertControls;
}

void PowerupItems::BroadcastTimedPowerup(boost::shared_ptr<Player> player, eGlide_Timed_Powerup_ID powerupId, int time)
{
    if (!player->mLevel->mIsClientSide)
    {
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)
            ->broadcastToAllExceptTeam(
                player, boost::shared_ptr<ClientboundPowerupPacket>(new ClientboundPowerupPacket(powerupId, time)));
    }
}

void PowerupItems::ActivateTimedPowerup(boost::shared_ptr<Player> player, eGlide_Timed_Powerup_ID powerupId, int time)
{
    if (player->mLevel->mIsClientSide && !player->isSpectator())
    {
        if (IsDebuff(powerupId) && player->mAbilities.mInvulnerable)
        {
            return;
        }
        player->SetPowerupTicks(powerupId, time);
    }
}

void PowerupItems::DoTimedPowerup(boost::shared_ptr<Player> player, eGlide_Timed_Powerup_ID powerupId, int time)
{
    if (IsDebuff(powerupId))
    {
        BroadcastTimedPowerup(player, powerupId, time);
    }
    else
    {
        ActivateTimedPowerup(player, powerupId, time);
    }
}

void PowerupItems::DoSetInvulnerable(boost::shared_ptr<Player> player, int time)
{
    if (!player->mLevel->mIsClientSide)
    {
        player->mAbilities.mInvulnerable = true;
        PlayerList* players = gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers;
        players->broadcastAll(boost::shared_ptr<ClientboundPlayerAbilitiesPacket>(
            new ClientboundPlayerAbilitiesPacket(&player->mAbilities, player->getId())));
        player->SetPowerupTicks(eGlide_Timed_Powerup_ID_Invulnerable, time);
    }
}

void PowerupItems::UsePowerupItem(boost::shared_ptr<Player> player, int itemId)
{
    switch (itemId)
    {
    case ItemId::BLAZE_POWDER:
    {
        DoLargeBoost(player);
    }
    break;
    case BlockId::REDSTONE_TORCH:
    {
        DoSmallBoost(player);
    }
    break;
    case BlockId::TNT:
    {
        DoDropTNT(player);
    }
    break;
    case ItemId::EMERALD:
    {
        DoGainPoint(player);
    }
    break;
    case ItemId::FEATHER:
    {
        DoTimedPowerup(player, eGlide_Timed_Powerup_ID_SlowFall, 0x3c);
    }
    break;
    case ItemId::COMPASS:
    {
        DoTimedPowerup(player, eGlide_Timed_Powerup_ID_InvertControls, 0x3c);
    }
    break;
    case ItemId::GOLDEN_APPLE:
    {
        DoSetInvulnerable(player, 100);
    }
    break;
    }
}
