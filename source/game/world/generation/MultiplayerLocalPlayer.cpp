#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/EntityType.hpp"

#include "game/item/ItemInstance.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/Stat.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Input.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/core/GameSettings.hpp"
#include "game/network/TextureChangePacket.hpp"
#include "game/network/TextureAndGeometryChangePacket.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

MultiplayerLocalPlayer::MultiplayerLocalPlayer(Minecraft* minecraft, Level* level, User* user,
                                               ClientPacketListener* packetListener)
    : LocalPlayer(minecraft, level, user, level->mDimension->getType()->getId())
{
    LivingEntity::fjDerivedCtorCalls();
    mPacketListener = packetListener;
    mHurt = false;
    mScoreboardWait = -1;
    mHasSentOpenInventory = false;
    mHasSentRidingJump = false;
    mReceivedRespawn = false;
    mField0xa1b = false;
    mLastSentStat = 0;
    mBounceHeight = 0;
    mFreeDiverTicks = 0;
}

MultiplayerLocalPlayer::~MultiplayerLocalPlayer()
{
    if (mBounceHeight != 0)
    {
        delete mBounceHeight;
        mBounceHeight = 0;
    }
}

MultiPlayerLevel* MultiplayerLocalPlayer::GetLevel()
{
    return dynamic_cast<MultiPlayerLevel*>(mLevel);
}

bool MultiplayerLocalPlayer::hurt(const DamageSource* source, float damage)
{
    return false;
}

void MultiplayerLocalPlayer::heal(float amount)
{
}

void MultiplayerLocalPlayer::fjResetBounceHeight()
{
    delete mBounceHeight;
    mBounceHeight = 0;
}

void MultiplayerLocalPlayer::tick()
{
    if (!mLevel->hasChunkAt(BlockPos(Mth::floor(mX), 0, Mth::floor(mZ))))
        return;
#ifdef MINECRAFT_PC
    ClientMasterGameMode* master = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (master != NULL && master->ServerIsWaitingOnUsLoading() && !CGameNetworkManager::sInstance.IsHost())
        return;
#endif
    LocalPlayer::tick();

    if (isAlive() && (gMinecraftServer == NULL || !gMinecraftServer->IsServerPaused()))
    {
        if (isUnderLiquid(Material::WATER))
        {
            if (mFreeDiverTicks == 0)
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::AMBIENT_UNDERWATER_ENTER,
                                       SoundSource::eSoundSource_Ambient, 1.0f, 1.0f, false, 16.0f);
            }
            ++mFreeDiverTicks;
            int seconds = mFreeDiverTicks / 20;
            if (seconds > 120)
                awardStat(GenericStats::freeDiver(), GenericStats::param_freeDiver(seconds));
        }
        else
        {
            if (mFreeDiverTicks != 0)
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::AMBIENT_UNDERWATER_EXIT,
                                       SoundSource::eSoundSource_Ambient, 1.0f, 1.0f, false, 16.0f);
            }
            mFreeDiverTicks = 0;
        }
    }

    if (mBounceHeight != 0)
    {
        int heightGained = (int)(mY - *mBounceHeight);
        if (heightGained >= 30)
            awardStat(GenericStats::trampoline(), GenericStats::param_trampoline(heightGained));
        if (mYd <= 0.0)
            fjResetBounceHeight();
    }

    if (mScoreboardWait < 0)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        {
            ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
            if (gameMode != NULL && gameMode->IsRoundPlaying())
            {
                bool endZoneEntered = GetLevel()->GetClientScoreboard()->GetEndZoneEntered(getSUID(), -1);
                bool gliding = isFallFlying() && !isInWater() && !isInLava();
                if (endZoneEntered && !isSpectator() && !gliding)
                    mScoreboardWait = 40;
            }
        }
    }
    else if (--mScoreboardWait == 0)
    {
        SetWaitingForScoreboard(true);
        StartSpectating();
    }
}

boost::shared_ptr<ItemEntity> MultiplayerLocalPlayer::drop()
{
    {
        boost::shared_ptr<Packet> packet(new ServerboundPlayerActionPacket(ServerboundPlayerActionPacket::DROP_ITEM,
                                                                           BlockPos(0, 0, 0), Direction::DOWN, 0));
        mPacketListener->send(packet);
    }
    return boost::shared_ptr<ItemEntity>();
}

not_null_ptr<ItemInstance> MultiplayerLocalPlayer::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    return ItemInstance::EMPTY;
}

void MultiplayerLocalPlayer::respawn()
{
    if (mPacketListener == NULL)
        return;
    boost::shared_ptr<Packet> packet(
        new ServerboundClientCommandPacket(ServerboundClientCommandPacket::PERFORM_RESPAWN));
    mPacketListener->send(packet);
}

void MultiplayerLocalPlayer::actuallyHurt(const DamageSource* source, float damage)
{
    if (!isInvulnerableTo(source) || source->isBypassInvul())
        setHealth(getHealth() - damage);
}

void MultiplayerLocalPlayer::completeUsingItem()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!mUseItem->isEmpty() && minecraft->mGameModes[mUserIndex] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[mUserIndex]->getTutorial();
        if (tutorial != NULL)
            tutorial->completeUsingItem(mUseItem);
    }
    if (mUseItem->isEmpty())
        mUseItem = getCarriedItem();
    LivingEntity::completeUsingItem();
}

void MultiplayerLocalPlayer::onEffectAdded(MobEffectInstance* effect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mUserIndex] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[mUserIndex]->getTutorial();
        if (tutorial != NULL)
            tutorial->onEffectChanged(effect->getEffect(), false);
    }
    LivingEntity::onEffectAdded(effect);
}

void MultiplayerLocalPlayer::onEffectUpdated(MobEffectInstance* effect, bool attributesChanged)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mUserIndex] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[mUserIndex]->getTutorial();
        if (tutorial != NULL)
            tutorial->onEffectChanged(effect->getEffect(), false);
    }
    LivingEntity::onEffectUpdated(effect, attributesChanged);
}

void MultiplayerLocalPlayer::onEffectRemoved(MobEffectInstance* effect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mUserIndex] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[mUserIndex]->getTutorial();
        if (tutorial != NULL)
            tutorial->onEffectChanged(effect->getEffect(), true);
    }
    LivingEntity::onEffectRemoved(effect);
}

void MultiplayerLocalPlayer::clientSideCloseContainer()
{
    mInventory->setCarried(ItemInstance::EMPTY);
    LocalPlayer::clientSideCloseContainer();
}

void MultiplayerLocalPlayer::closeContainer()
{
    {
        boost::shared_ptr<Packet> packet(new ServerboundContainerClosePacket(mContainerMenu->m_containerId));
        mPacketListener->send(packet);
    }
    MultiplayerLocalPlayer::clientSideCloseContainer();
}

void MultiplayerLocalPlayer::hurtTo(float amount, TelemetryDamageSource source)
{
    if (mHurt)
        LocalPlayer::hurtTo(amount, source);
    else
    {
        setHealth(amount);
        mHurt = true;
    }
}

void MultiplayerLocalPlayer::awardStat(Stat* stat, arrayWithLength<unsigned char> params)
{
    if (stat != 0 && !stat->isAwardedFromServer())
    {
        LocalPlayer::awardStat(stat, params);
        return;
    }
    delete[] params.data;
}

void MultiplayerLocalPlayer::awardStatFromServer(Stat* stat, arrayWithLength<unsigned char> params)
{
    if (stat != 0 && stat->isAwardedFromServer())
    {
        LocalPlayer::awardStat(stat, params);
        return;
    }
    delete[] params.data;
}

bool MultiplayerLocalPlayer::isLocalPlayer()
{
    return true;
}

void MultiplayerLocalPlayer::onSlimeBlockBounce()
{
    delete mBounceHeight;
    mBounceHeight = new double(mY);
}

void MultiplayerLocalPlayer::sendRidingJump()
{
    mPacketListener->send(boost::shared_ptr<Packet>(
        new ServerboundPlayerCommandPacket(shared_from_this(), ServerboundPlayerCommandPacket::RIDING_JUMP,
                                           static_cast<int>(getJumpRidingScale() * 100.0f))));
}

void MultiplayerLocalPlayer::sendOpenInventory()
{
    mPacketListener->send(boost::shared_ptr<Packet>(
        new ServerboundPlayerCommandPacket(shared_from_this(), ServerboundPlayerCommandPacket::OPEN_INVENTORY)));
}

bool MultiplayerLocalPlayer::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    bool wasRiding = isPassenger();
    bool result = LocalPlayer::startRiding(vehicle, force);
    bool riding = isPassenger();
    if (riding)
    {
        int challenge = 0;
        if (getVehicle() != NULL)
        {
            unsigned long long type = getVehicle()->GetType();
            if (type == EntityType::PIG)
                challenge = 0x12;
            else if (type == EntityType::MINECART_RIDEABLE || type == EntityType::MINECART)
                challenge = 0x10;
            else if (type == EntityType::BOAT)
                challenge = 0x11;
        }
        telemetryManagerInstance->RecordEnemyKilledOrOvercome(mUserIndex, (int)mX, (int)mY, (int)mZ, 0, 0, 0,
                                                              static_cast<ETelemetryChallenges>(challenge));
    }
    else if (wasRiding)
    {
        setSneaking(false);
        mInput->mSneaking = false;
    }
    updateRichPresence();

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mUserIndex] != NULL)
    {
        if (!wasRiding)
        {
            if (riding)
            {
                Tutorial* tutorial = minecraft->mGameModes[mUserIndex]->getTutorial();
                if (tutorial != NULL)
                    tutorial->onRideEntity(vehicle);
            }
        }
        else if (!riding)
        {
            Tutorial* tutorial = minecraft->mGameModes[mUserIndex]->getTutorial();
            if (tutorial != NULL)
                tutorial->changeTutorialState(eTutorial_State_Gameplay, NULL);
        }
    }
    return result;
}

void MultiplayerLocalPlayer::StopSleeping()
{
    mPacketListener->send(boost::shared_ptr<Packet>(
        new ServerboundPlayerCommandPacket(shared_from_this(), ServerboundPlayerCommandPacket::STOP_SLEEPING)));
}

void MultiplayerLocalPlayer::setAndBroadcastCustomSkin(unsigned int skin)
{
    int oldSkin = mCustomSkin;
    Player::setCustomSkin(skin);
    if (mCustomSkin == oldSkin)
        return;

    mPacketListener->send(boost::shared_ptr<Packet>(
        new TextureAndGeometryChangePacket(shared_from_this(), GameSettings::GetPlayerSkinName(mUserIndex))));
}

void MultiplayerLocalPlayer::setAndBroadcastCustomCape(unsigned int cape)
{
    int oldCape = mCustomCape;
    Player::setCustomCape(cape);
    if (mCustomCape == oldCape)
        return;

    mPacketListener->send(boost::shared_ptr<Packet>(
        new TextureChangePacket(shared_from_this(), TextureChangePacket::eTextureChangeType_Cape,
                                GameSettings::GetPlayerCapeName(mUserIndex))));
}
