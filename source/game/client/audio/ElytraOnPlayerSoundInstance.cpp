#include "game/client/audio/ElytraOnPlayerSoundInstance.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

ElytraOnPlayerSoundInstance::ElytraOnPlayerSoundInstance(boost::shared_ptr<Player> player)
    : AbstractTickableSoundInstance(SoundEvents::ELYTRA_FLYING, SoundSource::eSoundSource_Players)
{
    this->player = player;
    this->looping = true;
    this->delay = 0;
    this->volume = 1.0f;
    this->ticks = 0;
    this->warningSoundDelay = 0;
    this->warning = false;
}

ElytraOnPlayerSoundInstance::ElytraOnPlayerSoundInstance(const ElytraOnPlayerSoundInstance& other)
    : SoundInstance(other),
      AbstractTickableSoundInstance(other),
      ticks(other.ticks),
      player(other.player),
      warningSoundDelay(other.warningSoundDelay),
      warning(other.warning)
{
}

ElytraOnPlayerSoundInstance::~ElytraOnPlayerSoundInstance()
{
}

SoundInstance* ElytraOnPlayerSoundInstance::getDRCClone() const
{
    ElytraOnPlayerSoundInstance* clone = new ElytraOnPlayerSoundInstance(*this);
    clone->drcClone = true;
    AbstractTickableSoundInstance* result = clone;
    return result;
}

void ElytraOnPlayerSoundInstance::tick()
{
    ++this->ticks;

    if (this->player->mRemoved)
    {
        this->stopped = true;
        return;
    }

    if (this->ticks > 20 && !this->player->isFallFlying())
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->player->mX);
    this->y = static_cast<float>(this->player->mY);
    this->z = static_cast<float>(this->player->mZ);

    float speed = 0.0f;
    if (!this->player->PositionLocked())
    {
        speed = Mth::sqrt(this->player->mXd * this->player->mXd + this->player->mZd * this->player->mZd
                          + this->player->mYd * this->player->mYd);
    }

    float scaledSpeed = speed * 0.5f;

    if (this->player->getElytraProximityWarning())
        this->warning = true;

    if (this->warning)
    {
        float warningScale = static_cast<float>(this->warningSoundDelay / 10.0) + 1.0f;
        scaledSpeed = scaledSpeed * warningScale;
        this->warningSoundDelay = this->warningSoundDelay < 10 ? this->warningSoundDelay + 1 : 10;
        if (!this->player->getElytraProximityWarning() && warningScale == 2.0f)
            this->warning = false;
    }
    else if (this->warningSoundDelay > 0)
    {
        float warningScale = static_cast<float>(this->warningSoundDelay / 10.0) + 1.0f;
        scaledSpeed = scaledSpeed * warningScale;
        --this->warningSoundDelay;
    }

    const float maxVolume = 0.8f;
    float newVolume = 0.0f;
    if (speed >= 0.01)
        newVolume = Mth::clamp(scaledSpeed * scaledSpeed, 0.0f, maxVolume);

    if (this->ticks < 20)
    {
        this->volume = 0.0f;
        this->pitch = 1.0f;
    }
    else
    {
        if (this->ticks < 40)
            newVolume = newVolume * static_cast<float>((this->ticks - 20) / 20.0);

        this->volume = newVolume;
        if (newVolume > maxVolume)
            this->pitch = (newVolume - maxVolume) + 1.0f;
        else
            this->pitch = 1.0f;
    }

    if (eTYPE_DERIVED_FROM(EntityType::LOCAL_PLAYER, this->player->GetType()))
    {
        boost::shared_ptr<LocalPlayer> localPlayer = boost::static_pointer_cast<LocalPlayer>(this->player);
        if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        {
            if (gConsoleUIController.IsSceneInStack(localPlayer->mUserIndex, EUIScene_PauseMenu))
                this->volume = 0.0f;
#ifdef MINECRAFT_PC
            if (gConsoleUIController.IsSceneInStack(localPlayer->mUserIndex, EUIScene_PcGlideSections))
                this->volume = 0.0f;
#endif
        }
    }
}
