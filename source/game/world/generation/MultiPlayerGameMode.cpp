#include "game/world/generation/MultiPlayerGameMode.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModViews.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#endif
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/LevelRenderer.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/minigame/FullTutorial.hpp"
#include "game/minigame/GameModeTutorial.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/TeamInvite.hpp"
#include "game/core/System.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/CraftItemPacket.hpp"
#include "game/network/DebugOptionsPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ServerboundContainerButtonClickPacket.hpp"
#include "game/network/ServerboundContainerClickPacket.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/network/ServerboundInteractPacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundUseItemOnPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"
#include "game/core/Direction.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/PlayerTeam.hpp"

MultiPlayerGameMode::MultiPlayerGameMode(int playerId, ClientPacketListener* connection, int userIndex,
                                         bool primaryPlayer, bool splitScreen)
{
    mDestroyBlockPos = BlockPos(-1, -1, -1);
    mDestroyItem = ItemInstance::EMPTY;
    mGameType = GameType::SURVIVAL;
    mDestroying = false;
    mDestroyProgress = 0.0f;
    mDestroyTicks = 0;
    mDestroyDelay = 0;
    mConnection = connection;
    mPlayerId = playerId;
    mPrimaryPlayer = primaryPlayer;
    mCarriedItemSlot = 0;
    mGameModeConstraints = new GameModeConstraints(playerId);
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) && !splitScreen)
    {
        if (mPrimaryPlayer)
        {
            mTutorial = new FullTutorial(playerId, mGameModeConstraints, false);
            mMinecraft->playerStartedTutorial(playerId);
        }
        else
        {
            mTutorial = new Tutorial(playerId, mGameModeConstraints, false, false);
        }
    }
    else
    {
        mTutorial = new GameModeTutorial(playerId, mGameModeConstraints, userIndex);
    }
    mTeamInvites = std::vector<TeamInvite*>();
    mSpectateSoundAllowance = 5.0f;
    mSpectateSoundTime = System::processTimeInMilliSecs();
}

MultiPlayerGameMode::~MultiPlayerGameMode()
{
    delete mTutorial;
    delete mGameModeConstraints;
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        delete *it;
}

GameModeConstraints* MultiPlayerGameMode::getGameModeConstraints()
{
    return mGameModeConstraints;
}
boost::shared_ptr<MultiplayerLocalPlayer> MultiPlayerGameMode::createPlayer(Level* level)
{
    return boost::shared_ptr<MultiplayerLocalPlayer>(
        new MultiplayerLocalPlayer(mMinecraft, level, mMinecraft->mUser, mConnection));
}

void MultiPlayerGameMode::initPlayer(boost::shared_ptr<Player> player)
{
    player->mYRot = -180.0f;
}

void MultiPlayerGameMode::adjustPlayer(boost::shared_ptr<Player> player)
{
    mGameType->updatePlayerAbilities(&player->mAbilities);
}

#ifdef MINECRAFT_PC
bool MultiPlayerGameMode::FaceAim(float* savedYRot, float* savedXRot)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mPlayer;
    float yRot = 0.0f;
    float xRot = 0.0f;
    if (player == NULL || !PcModViewsAimRotation(player.get(), &yRot, &xRot))
        return false;
    *savedYRot = player->mYRot;
    *savedXRot = player->mXRot;
    player->mYRot = yRot;
    player->mXRot = xRot;
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundMovePlayerPacket::Rot(yRot, xRot, player->mOnGround, player->mAbilities.mFlying)));
    player->mLastSentYRot = yRot;
    player->mLastSentXRot = xRot;
    return true;
}

void MultiPlayerGameMode::EndFaceAim(float savedYRot, float savedXRot)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mPlayer;
    if (player == NULL)
        return;
    player->mYRot = savedYRot;
    player->mXRot = savedXRot;
}

namespace
{
class FacingAim
{
public:
    FacingAim(MultiPlayerGameMode* gameMode, bool face)
        : mGameMode(gameMode), mYRot(0.0f), mXRot(0.0f), mFacing(face && gameMode->FaceAim(&mYRot, &mXRot))
    {
    }
    ~FacingAim()
    {
        if (mFacing)
            mGameMode->EndFaceAim(mYRot, mXRot);
    }

private:
    MultiPlayerGameMode* mGameMode;
    float mYRot;
    float mXRot;
    bool mFacing;
};
}
#endif

void MultiPlayerGameMode::ensureHasSentCarriedItem()
{
    const int slot = mMinecraft->mPlayer->mInventory->m_selected;
    if (slot == mCarriedItemSlot)
        return;

    mCarriedItemSlot = slot;
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundSetCarriedItemPacket(slot)));
}

InteractionResult::EInteractionResult MultiPlayerGameMode::_useItemOn(boost::shared_ptr<Player> player, Level* level,
                                                                      const BlockPos& pos, const Direction* direction,
                                                                      Vec3* hit, InteractionHand::EInteractionHand hand,
                                                                      bool simulate, bool* used)
{
    const float hitX = static_cast<float>(hit->x - pos.x);
    const float hitY = static_cast<float>(hit->y - pos.y);
    const float hitZ = static_cast<float>(hit->z - pos.z);
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (used != NULL)
        *used = false;

    if (!simulate)
        ensureHasSentCarriedItem();
    if (!mMinecraft->mLevel->getWorldBorder()->isWithinBounds(pos))
        return InteractionResult::eInteractionResult_Fail;

    InteractionResult::EInteractionResult result = InteractionResult::eInteractionResult_Pass;
    bool useItem = true;
    if (!isSpectator())
    {
        const BlockState* state = level->getBlockState(pos);
        bool tryBlock = !(player->isSneaking()
                          && (!player->getMainHandItem()->isEmpty() || !player->getOffhandItem()->isEmpty()));

        if (tryBlock && player->isAllowedToUse(state->getBlock()))
        {
            if (simulate)
            {
                if (state->getBlock() == Blocks::jukebox || state->getBlock() == Blocks::bed)
                {
                    if (state->getBlock()->TestUse(level, pos, state, player))
                        return InteractionResult::eInteractionResult_Success;
                    if (state->getBlock() == Blocks::bed)
                        return InteractionResult::eInteractionResult_Fail;
                }
                else if (state->getBlock()->TestUse())
                {
                    return InteractionResult::eInteractionResult_Success;
                }
            }
            else if (state->getBlock()->use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, false))
            {
                result = InteractionResult::eInteractionResult_Success;
                useItem = false;
            }
        }

        if (useItem)
        {
            BlockItem* blockItem = dynamic_cast<BlockItem*>(item->getItem());
            if (blockItem != NULL)
            {
                if (!MayPlace(pos.relative(direction), blockItem)
                    || !blockItem->mayPlace(level, pos, direction, player, item))
                    return InteractionResult::eInteractionResult_Fail;
            }
        }
    }

    if (useItem)
    {
        result = isSpectator() ? InteractionResult::eInteractionResult_Success
                               : InteractionResult::eInteractionResult_Pass;
        if (!isSpectator() && !item->isEmpty())
        {
            if (player->getCooldowns()->isOnCooldown(item->getItem()))
                return InteractionResult::eInteractionResult_Pass;

            if (player->isAllowedToUse(item))
            {
                if (!isCreative())
                {
                    result = item->useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
                }
                else
                {
                    const int auxValue = item->getAuxValue();
                    const int count = item->getCount();
                    result = item->useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
                    item->setAuxValue(auxValue);
                    item->setCount(count);
                }
                if (result == InteractionResult::eInteractionResult_Success && used != NULL)
                    *used = true;
            }
        }
    }

    if (!simulate && result != InteractionResult::eInteractionResult_Pass)
        mConnection->send(
            boost::shared_ptr<Packet>(new ServerboundUseItemOnPacket(pos, direction, hand, hitX, hitY, hitZ)));
    return result;
}


bool MultiPlayerGameMode::GetTutorialsEnabled()
{
    bool enabled = true;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (!GetMiniGame()->GetTutorialsEnabled())
            enabled = false;
    }
    return enabled;
}
InteractionResult::EInteractionResult MultiPlayerGameMode::useItemOn(boost::shared_ptr<Player> player, Level* level,
                                                                     const BlockPos& pos, const Direction* direction,
                                                                     Vec3* hit, InteractionHand::EInteractionHand hand,
                                                                     bool simulate, bool* used)
{
#ifdef MINECRAFT_PC
    FacingAim facing(this, !simulate);
#endif
    not_null_ptr<ItemInstance> startingItem = player->getItemInHand(hand);
    bool trackStartingCount = false;
    int startingCount = 0;
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
    {
        mTutorial->useItemOn(level, startingItem, pos, simulate);
        mTutorial->useItemOn(startingItem, simulate);
        if (!simulate && !startingItem->isEmpty())
        {
            trackStartingCount = true;
            startingCount = startingItem->getCount();
        }
    }
    const InteractionResult::EInteractionResult result = MultiPlayerGameMode::_useItemOn(player, level, pos, direction,
                                                                                         hit, hand, simulate, used);
    if (!simulate && !mTutorial->tasksChecked)
    {
        if (GetTutorialsEnabled() && result != InteractionResult::eInteractionResult_Success && trackStartingCount)
        {
            if (startingItem->getCount() < startingCount)
                mTutorial->useItemOn(startingItem, false);
        }
    }
    return result;
}

bool MultiPlayerGameMode::hasMissTime()
{
    return !isCreative();
}

void MultiPlayerGameMode::attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity)
{
#ifdef MINECRAFT_PC
    FacingAim facing(this, true);
#endif
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
        mTutorial->attack(player, entity);

    ensureHasSentCarriedItem();
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundInteractPacket(entity)));

    if (!isSpectator())
    {
        player->attack(entity);
        player->resetAttackStrengthTicker();
    }
}

InteractionResult::EInteractionResult MultiPlayerGameMode::interact(boost::shared_ptr<Player> player,
                                                                    boost::shared_ptr<Entity> entity,
                                                                    not_null_ptr<ItemInstance>,
                                                                    InteractionHand::EInteractionHand hand)
{
    ensureHasSentCarriedItem();
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundInteractPacket(entity, hand)));
    if (!isSpectator())
        return static_cast<InteractionResult::EInteractionResult>(player->interactOn(entity, hand));
    return InteractionResult::eInteractionResult_Pass;
}

InteractionResult::EInteractionResult MultiPlayerGameMode::interactAt(boost::shared_ptr<Player> player,
                                                                      boost::shared_ptr<Entity> entity,
                                                                      const HitResult* hit, not_null_ptr<ItemInstance>,
                                                                      InteractionHand::EInteractionHand hand)
{
    ensureHasSentCarriedItem();
    Vec3* relative = Vec3::newTemp(hit->mLocation->x - entity->mX, hit->mLocation->y - entity->mY,
                                   hit->mLocation->z - entity->mZ);
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundInteractPacket(entity, hand, relative)));
    if (mGameType != GameType::SPECTATOR)
        return entity->interactAt(player, relative, hand);
    return InteractionResult::eInteractionResult_Pass;
}
bool MultiPlayerGameMode::hasInfiniteItems()
{
    return isCreative();
}
void MultiPlayerGameMode::handlePickItem(const boost::shared_ptr<ItemInstance>& item)
{
    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeItem(not_null_ptr<ItemInstance>(item));
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundCustomPayloadPacket(ServerboundCustomPayloadPacket::PICK_ITEM_PACKET, bytes.toByteArray())));
}
bool MultiPlayerGameMode::isInputAllowed(int mapping)
{
    bool allowed = true;
    if (GetTutorialsEnabled() && !mTutorial->tasksChecked && !mTutorial->popupPending
        && !mGameModeConstraints->isInputAllowed(mapping))
        allowed = false;

    bool positionOk = true;
    switch (mapping)
    {
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x20:
        if (mMinecraft->mLocalPlayers[mPlayerId]->PositionLocked())
        {
            positionOk = false;
            break;
        }
    case 0x27:
    case 0x29:
        if (mMinecraft->mLocalPlayers[mPlayerId]->IsSpectatingOtherPlayer())
            positionOk = false;
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f:
        if (mMinecraft->mLocalPlayers[mPlayerId]->GetThirdPersonView(false) == 3)
            positionOk = false;
        break;

    case 0x21:
        if (Minecraft::InMiniGame(eMiniGameId_Any, true) && mMinecraft->mLocalPlayers[mPlayerId]->PositionLocked())
            positionOk = false;
        break;

    case 0x22:
        break;

    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x28:
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        {
            const bool positionLocked = mMinecraft->mLocalPlayers[mPlayerId]->PositionLocked();
            const bool lockedInventory = Minecraft::GetInstance()->GetMiniGame()->LockedInventory();
            const bool inScoringState = Minecraft::GetInstance()->GetClientMasterGameMode()->IsInScoringState();
            if (positionLocked || lockedInventory || inScoringState)
                positionOk = false;
        }
        break;

    default:
        break;
    }

    return allowed && positionOk;
}
bool MultiPlayerGameMode::destroyBlock(const BlockPos& pos)
{
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
    {
        const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
        mTutorial->destroyBlock(state->getBlock());
    }

    not_null_ptr<ItemInstance> heldItem = mMinecraft->mPlayer->getMainHandItem();
    int oldDamage = 0;
    if (!heldItem->isEmpty())
        oldDamage = heldItem->getDamageValue();

    bool result = _destroyBlock(pos);

    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
    {
        if (heldItem->isDamageableItem())
        {
            const int maxDamage = heldItem->getMaxDamage();
            const int newDamage = heldItem->getDamageValue();
            if (oldDamage < newDamage && maxDamage / 2 < newDamage)
                mTutorial->itemDamaged(heldItem);
        }
    }

    return result;
}
void MultiPlayerGameMode::creativeDestroyBlock(Minecraft* minecraft, MultiPlayerGameMode* gameMode, const BlockPos& pos,
                                               const Direction* direction)
{
    if (!minecraft->mLevel->extinguishFire(minecraft->mPlayer, pos, direction))
        gameMode->destroyBlock(pos);
}
bool MultiPlayerGameMode::sameDestroyTarget(const BlockPos& pos)
{
    not_null_ptr<ItemInstance> heldItem = mMinecraft->mPlayer->getMainHandItem();
    bool result = false;
    if (pos.equals(mDestroyBlockPos))
    {
        if (heldItem->getItem() == mDestroyItem->getItem() && ItemInstance::tagMatches(heldItem, mDestroyItem))
        {
            if (heldItem->isDamageableItem() || heldItem->getAuxValue() == mDestroyItem->getAuxValue())
                result = true;
        }
    }
    return result;
}
bool MultiPlayerGameMode::continueDestroyBlock(const BlockPos& pos, const Direction* direction)
{
    if (!mMinecraft->mPlayer->isAllowedToMine())
        return false;

    ensureHasSentCarriedItem();

    if (mDestroyDelay > 0)
    {
        --mDestroyDelay;
        return true;
    }

    if (isCreative() && mMinecraft->mLevel->getWorldBorder()->isWithinBounds(pos))
    {
        mDestroyDelay = 5;
        mConnection->send(boost::shared_ptr<Packet>(
            new ServerboundPlayerActionPacket(ServerboundPlayerActionPacket::START_DESTROY_BLOCK, pos, direction, 0)));
        creativeDestroyBlock(mMinecraft, this, pos, direction);
        return true;
    }

    bool result;
    if (!sameDestroyTarget(pos))
    {
        result = startDestroyBlock(pos, direction);
    }
    else
    {
        const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
        Block* block = state->getBlock();
        if (block->mMaterial == Material::AIR)
        {
            mDestroying = false;
            return false;
        }

        if (isSpectator() && Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation())
            mDestroyProgress += 0.02f;
        else
            mDestroyProgress += state->getDestroyProgress(mMinecraft->mPlayer, mMinecraft->mPlayer->mLevel, pos);

        if ((mDestroyTicks & 3) == 0)
        {
            const SoundType* soundType = block->getSoundType();
            const SoundEvent* hitSound = soundType->getHitSound();
            const float volume = (block->mSoundType->getVolume() + 1.0f) * 0.125f;
            const float pitch = block->mSoundType->getPitch() * 0.5f;
            mMinecraft->mSoundEngine->play(hitSound, static_cast<float>(pos.x) + 0.5f, static_cast<float>(pos.y) + 0.5f,
                                           static_cast<float>(pos.z) + 0.5f, volume, pitch, 0);
        }
        ++mDestroyTicks;

        if (mDestroyProgress >= 1.0f)
        {
            mDestroying = false;
            mConnection->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
                ServerboundPlayerActionPacket::STOP_DESTROY_BLOCK, pos, direction, 0)));
            destroyBlock(pos);
            mDestroyTicks = 0;
            mDestroyDelay = 5;
            mDestroyProgress = 0.0f;
        }

        mMinecraft->mLevel->destroyBlockProgress(mMinecraft->mPlayer->getId(), mDestroyBlockPos,
                                                 static_cast<int>(mDestroyProgress * 10.0f) - 1);
        result = true;
    }
    return result;
}
void MultiPlayerGameMode::stopDestroyBlock()
{
    if (!mDestroying)
        return;

    mConnection->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
        ServerboundPlayerActionPacket::ABORT_DESTROY_BLOCK, mDestroyBlockPos, Direction::DOWN, 0)));

    mDestroyProgress = 0.0f;
    mDestroying = false;

    mMinecraft->mLevel->destroyBlockProgress(mMinecraft->mPlayer->getId(), mDestroyBlockPos, -1);
    mMinecraft->mPlayer->resetAttackStrengthTicker();
}
Tutorial* MultiPlayerGameMode::getTutorial()
{
    return GetTutorialsEnabled() ? mTutorial : NULL;
}
bool MultiPlayerGameMode::isDestroying()
{
    return mDestroying;
}
void MultiPlayerGameMode::releaseUsingItem(boost::shared_ptr<Player> player)
{
#ifdef MINECRAFT_PC
    FacingAim facing(this, true);
#endif
    ensureHasSentCarriedItem();
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundPlayerActionPacket(ServerboundPlayerActionPacket::RELEASE_USE_ITEM, BlockPos::ZERO,
                                          Direction::DOWN, player->getUseItemRemainingTicks())));
    player->releaseUsingItem();
}
bool MultiPlayerGameMode::isServerControlledInventory()
{
    return mMinecraft->mPlayer->isPassenger()
           && eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, mMinecraft->mPlayer->getVehicle()->GetType());
}
boost::shared_ptr<MultiplayerLocalPlayer> MultiPlayerGameMode::GetLocalPlayer()
{
    return mMinecraft->mLocalPlayers[mPlayerId];
}
bool MultiPlayerGameMode::SpectateSoundEnabled()
{
    return Minecraft::InMiniGame(eMiniGameId_Any, true) && GetLocalPlayer()->SpectateAsOtherEntity()
           && !GetMiniGame()->IsBatSqueakDisabled();
}
float MultiPlayerGameMode::CalcSpectateSoundAllowance()
{
    const long long now = System::processTimeInMilliSecs();
    const float elapsed = static_cast<float>(now - static_cast<long long>(mSpectateSoundTime));
    const float allowance = mSpectateSoundAllowance + elapsed * 5.0f / 8000.0f;
    if (allowance > 5.0f)
        return 5.0f;
    return allowance;
}
bool MultiPlayerGameMode::CanMakeSpectateSound()
{
    if (!SpectateSoundEnabled())
        return false;
    const float elapsed = static_cast<float>(
        static_cast<long long>(System::processTimeInMilliSecs() - mSpectateSoundTime));
    return CalcSpectateSoundAllowance() >= 1.0 && elapsed > 300.0f;
}
void MultiPlayerGameMode::MakeSpectateSound()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = GetLocalPlayer();
    const unsigned long long entityType = player->GetSpectateEntity();
    const SoundEvent* sound;
    if (entityType == EntityType::VEX)
        sound = SoundEvents::VEX_CHARGE;
    else if (entityType == EntityType::PARROT)
        sound = SoundEvents::PARROT_AMBIENT;
    else if (entityType == EntityType::BAT)
        sound = SoundEvents::BAT_AMBIENT;
    else
        return;
    mSpectateSoundAllowance = CalcSpectateSoundAllowance() - 1.0f;
    mSpectateSoundTime = System::processTimeInMilliSecs();
    mMinecraft->mSoundEngine->play(sound, static_cast<float>(player->mX), static_cast<float>(player->mY),
                                   static_cast<float>(player->mZ), 0.5f, 0.0f, 0);
    player->mPacketListener->send(boost::shared_ptr<Packet>(new ClientboundSoundPacket(
        sound, SoundSource::eSoundSource_Players, player->mX, player->mY, player->mZ, 0.5f, 0.0f)));
}
bool MultiPlayerGameMode::isCutScene()
{
    return false;
}


void MultiPlayerGameMode::Follow(bool follow, bool immediate)
{
    Minecraft::GetInstance()->mLocalPlayers[mPlayerId]->Follow(follow);
    if (!follow)
        mMinecraft->mGui->clearMessages(mPlayerId);
    GameModePacket::FollowData data(follow, immediate);
    Minecraft::GetInstance()->mLocalPlayers[mPlayerId]->mPacketListener->send(
        GameModePacket::Create(GameModePacket::eMessage_Follow, data));
    if (follow && getTutorial() != NULL && !Minecraft::GetInstance()->GetClientMasterGameMode()->IsRoundEnding())
        getTutorial()->onGameModeChange(eTutorial_GameMode_Following);
}
void MultiPlayerGameMode::StartSpectating()
{
    GameModePacket::EmptyData data;
    Minecraft::GetInstance()->mLocalPlayers[mPlayerId]->mPacketListener->send(
        GameModePacket::Create(GameModePacket::eMessage_StartSpectating, data));
}
bool MultiPlayerGameMode::startDestroyBlock(const BlockPos& pos, const Direction* direction)
{
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
    {
        const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
        mTutorial->startDestroyBlock(mMinecraft->mPlayer->getMainHandItem(), state);
    }

    if (Minecraft::InMiniGame(eMiniGameId_Lobby, true))
    {
        mConnection->send(boost::shared_ptr<Packet>(
            new ServerboundPlayerActionPacket(ServerboundPlayerActionPacket::START_DESTROY_BLOCK, pos, direction, 0)));
        stopDestroyBlock();
        return false;
    }

    if (!mMinecraft->mPlayer->isAllowedToMine() || !MayDestroy(pos)
        || !mMinecraft->mLevel->getWorldBorder()->isWithinBounds(pos))
    {
        stopDestroyBlock();
        return false;
    }

    if (isCreative())
    {
        mConnection->send(boost::shared_ptr<Packet>(
            new ServerboundPlayerActionPacket(ServerboundPlayerActionPacket::START_DESTROY_BLOCK, pos, direction, 0)));
        creativeDestroyBlock(mMinecraft, this, pos, direction);
        mDestroyDelay = 5;
        return true;
    }

    if (mDestroying && sameDestroyTarget(pos))
        return true;

    if (mDestroying)
    {
        mConnection->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
            ServerboundPlayerActionPacket::ABORT_DESTROY_BLOCK, mDestroyBlockPos, direction, 0)));
    }

    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundPlayerActionPacket(ServerboundPlayerActionPacket::START_DESTROY_BLOCK, pos, direction, 0)));

    const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
    if (state->getMaterial() != Material::AIR && mDestroyProgress == 0.0f)
        state->getBlock()->attack(mMinecraft->mLevel, pos, mMinecraft->mPlayer);

    bool spectatorMining = false;
    if (isSpectator())
        spectatorMining = Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation();

    if (state->getMaterial() != Material::AIR && !spectatorMining
        && state->getDestroyProgress(mMinecraft->mPlayer, mMinecraft->mPlayer->mLevel, pos) >= 1.0f)
    {
        destroyBlock(pos);
        return true;
    }

    mDestroying = true;
    mDestroyBlockPos = pos;
    mDestroyItem = mMinecraft->mPlayer->getMainHandItem();
    mDestroyTicks = 0;
    mDestroyProgress = 0.0f;
    mMinecraft->mLevel->destroyBlockProgress(mMinecraft->mPlayer->getId(), pos,
                                             static_cast<int>(mDestroyProgress * 10.0f) - 1);
    return true;
}
bool MultiPlayerGameMode::canHurtPlayer()
{
    return isSurvival();
}
void MultiPlayerGameMode::tick()
{
    ensureHasSentCarriedItem();
    mGameModeConstraints->tick();
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
        mTutorial->tick();
}
InteractionResult::EInteractionResult MultiPlayerGameMode::useItem(boost::shared_ptr<Player> player, Level* level,
                                                                   InteractionHand::EInteractionHand hand,
                                                                   bool simulate)
{
#ifdef MINECRAFT_PC
    FacingAim facing(this, !simulate);
#endif
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);

    if (isSpectator())
        return InteractionResult::eInteractionResult_Pass;

    if (player->isAllowedToUse(item) != true)
        return InteractionResult::eInteractionResult_Pass;

    if (!simulate)
        ensureHasSentCarriedItem();

    InteractionResult::EInteractionResult result = InteractionResult::eInteractionResult_Pass;
    if (!simulate)
    {
        if (!player->getCooldowns()->isOnCooldown(item->getItem()))
        {
            int savedCount = item->getCount();
            InteractionResultHolder holder = item->use(level, player, hand);
            not_null_ptr<ItemInstance> resultItem = holder.getObject();

            if (resultItem != item || resultItem->getCount() != savedCount)
                player->setItemInHand(hand, resultItem);

            result = holder.getResult();
        }
    }
    else
    {
        bool canUse = item->TestUse(level, player, hand);
        result = canUse ? InteractionResult::eInteractionResult_Success : InteractionResult::eInteractionResult_Pass;
    }

    if (!simulate && result != InteractionResult::eInteractionResult_Pass)
        mConnection->send(boost::shared_ptr<Packet>(new ServerboundUseItemPacket(hand)));

    return result;
}
void MultiPlayerGameMode::SetGameType(const GameType* gameType)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mLocalPlayers[mPlayerId];
    if (!mMinecraft->GetClientMasterGameMode()->IsRoundEnding() && gameType != GameType::LOBBY
        && gameType == GameType::SPECTATOR && getTutorial() != 0)
    {
        eTutorial_GameMode mode = eTutorial_GameMode_SpectatingAsMob;
        if (player->SpectateAsInvisible())
            mode = eTutorial_GameMode_SpectatingInvisible;
        getTutorial()->onGameModeChange(mode);
    }
    mGameType = gameType;
    if (!player->isSleeping())
        player->UpdateSize();
}
const GameType* MultiPlayerGameMode::GetGameType()
{
    return mGameType;
}
void MultiPlayerGameMode::setLocalMode(const GameType* gameType)
{
    mGameType = gameType;
    mGameType->updatePlayerAbilities(&mMinecraft->mPlayer->mAbilities);
}
bool MultiPlayerGameMode::_destroyBlock(const BlockPos& pos)
{
    if (!MayDestroy(pos))
        return false;

    Level* level = mMinecraft->mLevel;
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (state->getMaterial() == Material::AIR)
        return false;

    level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getIdWithData(state));

    bool handled = block->playerWillDestroy(level, pos, state, mMinecraft->mLocalPlayers[mPlayerId]);
    if (!handled)
    {
        mMinecraft->mLevelRenderer->mDestroyedBlocks->destroyingBlockAt(level, pos);
        handled = level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE,
                                  false);
        if (handled)
            block->destroy(level, pos, state);
    }

    mDestroyBlockPos = BlockPos(mDestroyBlockPos.x, -1, mDestroyBlockPos.z);

    if (!isCreative())
    {
        not_null_ptr<ItemInstance> heldItem = mMinecraft->mPlayer->getMainHandItem();
        if (!heldItem->isEmpty())
        {
            heldItem->mineBlock(level, state, pos, mMinecraft->mPlayer);
            if (heldItem->isEmpty())
                mMinecraft->mPlayer->setItemInHand(InteractionHand::eInteractionHand_MainHand, ItemInstance::EMPTY);
        }
    }

    return handled;
}
double MultiPlayerGameMode::getPickRange()
{
    float creativeRange = 5.0f;
    float survivalRange = 4.5f;
    if (isCreative())
        return creativeRange;
    return survivalRange;
}
void MultiPlayerGameMode::handleTakeAllInvenetory(int containerId, int button, int start, int count,
                                                  boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    AbstractContainerMenu::ItemList items = player->mContainerMenu->getAllItems(start, count);
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundContainerClickPacket(
        containerId, start, button, ClickType::eClickType_QuickMove, items, static_cast<short>(uid), count)));
}
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleInventoryMouseClick(int containerId, int slot, int button,
                                                                               ClickType::eClickType clickType,
                                                                               boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    not_null_ptr<ItemInstance> item = player->mContainerMenu->clicked(slot, button, clickType, player);
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundContainerClickPacket(containerId, slot, button, clickType, item, static_cast<short>(uid))));
    return item;
}
void MultiPlayerGameMode::handleInventoryButtonClick(int containerId, int button)
{
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundContainerButtonClickPacket(containerId, button)));
}

void MultiPlayerGameMode::handleCreativeModeItemAdd(not_null_ptr<ItemInstance> item, int slot)
{
    if (!isCreative())
        return;
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundSetCreativeModeSlotPacket(slot, item)));
}

void MultiPlayerGameMode::handleCreativeModeItemDrop(not_null_ptr<ItemInstance> item)
{
    if (!isCreative())
        return;
    if (item->isEmpty())
        return;
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundSetCreativeModeSlotPacket(-1, item)));
}
bool MultiPlayerGameMode::hasExperience()
{
    return isSurvival();
}
bool MultiPlayerGameMode::hasFarPickRange()
{
    return isCreative();
}
not_null_ptr<ItemInstance> MultiPlayerGameMode::handleCraftItem(int recipe, int category,
                                                                boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    mConnection->send(boost::shared_ptr<Packet>(new CraftItemPacket(recipe, category, static_cast<short>(uid))));
    return GameMode::handleCraftItem(recipe, category, player);
}
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleStackedCraftItem(std::vector<std::pair<int, int> >& recipes,
                                                                            int category,
                                                                            boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    mConnection->send(boost::shared_ptr<Packet>(new CraftItemPacket(recipes, category, static_cast<short>(uid))));
    return GameMode::handleStackedCraftItem(recipes, category, player);
}
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleCustomCraftItem(std::vector<int>& recipes,
                                                                           boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    mConnection->send(boost::shared_ptr<Packet>(new CraftItemPacket(recipes, static_cast<short>(uid))));
    return GameMode::handleCustomCraftItem(recipes, player);
}
void MultiPlayerGameMode::handleDebugOptions(unsigned int option, boost::shared_ptr<Player> player)
{
    player->mDebugOptions = option;
    mConnection->send(boost::shared_ptr<Packet>(new DebugOptionsPacket(option)));
}
const MiniGameDef* MultiPlayerGameMode::GetMiniGame()
{
    return mMinecraft->GetMiniGame();
}
Level* MultiPlayerGameMode::GetLevel()
{
    return mMinecraft->mLevel;
}

boost::shared_ptr<Player> MultiPlayerGameMode::GetPlayer()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = GetLocalPlayer();
    return player;
}
void MultiPlayerGameMode::OnMouseDown()
{
}

std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInvite(unsigned int teamId)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if ((*it)->m_uTeamID == teamId)
            break;
    return it;
}

void MultiPlayerGameMode::SyncTeamInvite(TeamInvite* invite)
{
    std::vector<TeamInvite*>::iterator it = FindTeamInvite(invite->m_uTeamID);
    if (it == mTeamInvites.end())
        mTeamInvites.push_back(invite);
    else
        delete invite;
}

void MultiPlayerGameMode::RemoveTeamInvite(unsigned int teamId)
{
    std::vector<TeamInvite*>::iterator it = FindTeamInvite(teamId);
    if (it != mTeamInvites.end())
    {
        delete *it;
        mTeamInvites.erase(it);
    }
}

std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInviteFrom(std::wstring playerName)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if ((*it)->m_sFromPlayerName == playerName)
            break;
    return it;
}

bool MultiPlayerGameMode::IsTeamInvite(std::wstring playerName)
{
    if (GetPlayer()->getNetworkName() == playerName)
        return false;
    return FindTeamInviteFrom(playerName) != mTeamInvites.end();
}
void MultiPlayerGameMode::SendTeamInvite(std::wstring playerName)
{
    if (isSpectator())
        return;
    std::wstring name = playerName;
    GameModePacket::PlayerData data(name);
    GetLocalPlayer()->mPacketListener->send(GameModePacket::Create(GameModePacket::eMessage_SendTeamInvite, data));
}
std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInviteTo(std::wstring playerName)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if ((*it)->m_sToPlayerName == playerName)
            break;
    return it;
}
bool MultiPlayerGameMode::CanSendInvite(std::wstring playerName, PlayerUID playerUid)
{
    if (isSpectator())
        return false;

    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    PlayerTeam* myTeam = scoreboard->getPlayersTeam(GetPlayer()->getXuid());
    PlayerTeam* theirTeam = scoreboard->getPlayersTeam(playerUid);

    if (myTeam != 0 && theirTeam != 0 && myTeam->getPlayerCount() <= 1)
    {
        bool sameUid = GetPlayer()->getXuid() == playerUid;
        if (!sameUid && myTeam != theirTeam)
            return FindTeamInviteTo(playerName) == mTeamInvites.end();
    }
    return false;
}
bool MultiPlayerGameMode::CanLeaveTeam(PlayerUID uid)
{
    if (isSpectator())
        return false;

    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    PlayerTeam* ownTeam = scoreboard->getPlayersTeam(GetPlayer()->getXuid());
    PlayerTeam* otherTeam = scoreboard->getPlayersTeam(uid);
    if (ownTeam == NULL || otherTeam == NULL)
        return false;
    if (GetPlayer()->getXuid() == uid)
        return false;
    return ownTeam == otherTeam;
}
void MultiPlayerGameMode::AcceptTeamInvite(std::wstring playerName)
{
    std::vector<TeamInvite*>::iterator it = FindTeamInviteFrom(playerName);
    if (it == mTeamInvites.end())
        return;
    GameModePacket::TeamIDData data((*it)->m_uTeamID);
    GetLocalPlayer()->mPacketListener->send(GameModePacket::Create(GameModePacket::eMessage_AcceptTeamInvite, data));
}

void MultiPlayerGameMode::LeaveTeam()
{
    if (isSpectator())
        return;

    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    PlayerTeam* team = scoreboard->getPlayersTeam(GetPlayer()->getXuid());
    if (team != 0)
    {
        GameModePacket::EmptyData data;
        GetLocalPlayer()->mPacketListener->send(GameModePacket::Create(GameModePacket::eMessage_LeaveTeam, data));
    }
}
bool MultiPlayerGameMode::RecordLastCheckpoint(int checkpoint, int, bool, bool, bool)
{
    boost::shared_ptr<Player> player = GetPlayer();
    return mMinecraft->GetClientMasterGameMode()->SetLastCheckpointID(player->getSUID(), checkpoint, false);
}
