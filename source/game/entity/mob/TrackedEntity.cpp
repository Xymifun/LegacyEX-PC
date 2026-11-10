#include "game/entity/mob/TrackedEntity.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModHooks.hpp"
#endif
#include "game/entity/mob/EntityIO.hpp"
#include "game/core/EntityType.hpp"

#include <cstdlib>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/MinecartTNT.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ModifiableAttributeMap.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/Creature.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/entity/projectile/Fireball.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/ClientboundAddEntityPacket.hpp"
#include "game/network/ClientboundAddExperienceOrbPacket.hpp"
#include "game/network/ClientboundAddMobPacket.hpp"
#include "game/network/ClientboundAddPaintingPacket.hpp"
#include "game/network/ClientboundAddPlayerPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/network/ClientboundMoveEntityPacket.hpp"
#include "game/network/ClientboundPlayerSleepPacket.hpp"
#include "game/network/ClientboundRotateHeadPacket.hpp"
#include "game/network/ClientboundSetEntityDataPacket.hpp"
#include "game/network/ClientboundSetEntityLinkPacket.hpp"
#include "game/network/ClientboundSetEntityMotionPacket.hpp"
#include "game/network/ClientboundSetEquippedItemPacket.hpp"
#include "game/network/ClientboundSetPassengersPacket.hpp"
#include "game/network/ClientboundTeleportEntityPacket.hpp"
#include "game/network/ClientboundUpdateAttributesPacket.hpp"
#include "game/network/ClientboundUpdateMobEffectPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/MoveEntityPacketSmall.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"

static int sSmallPosRotPacketCount = 0;
static int sPosRotPacketCount = 0;
static int sSmallPosPacketCount = 0;
static int sPosPacketCount = 0;
static int sSmallRotPacketCount = 0;
static int sRotPacketCount = 0;

TrackedEntity::TrackedEntity(boost::shared_ptr<Entity> entity, int range, int maxRange, int updateInterval,
                             bool trackDelta)
    : mEntity(),
      mLastPassengers(),
      mSeenBy()
{
    mUpdatedPlayerVisibility = false;
    mPlayerEntitiesUpdated = false;
    mWasRiding = false;
    mTickCount = 0;
    mTeleportDelay = 0;
    mXap = 0.0;
    mYap = 0.0;
    mZap = 0.0;
    mEntity = entity;
    mRange = range;
    mTrackDelta = trackDelta;
    mMaxRange = maxRange;
    mUpdateInterval = updateInterval;
    mXp = Mth::floor(entity->mX * 32.0);
    mYp = Mth::floor(entity->mY * 32.0);
    mZp = Mth::floor(entity->mZ * 32.0);
    mXpu = entity->mX;
    mYpu = entity->mY;
    mZpu = entity->mZ;
    mYRotp = Mth::floor(entity->mYRot * 256.0f / 360.0f);
    mXRotp = Mth::floor(entity->mXRot * 256.0f / 360.0f);
    mYHeadRotp = Mth::floor(entity->getYHeadRot() * 256.0f / 360.0f);
    mWasOnGround = entity->mOnGround;
}

bool TrackedEntity::canBySeenBy(boost::shared_ptr<ServerPlayer> player)
{
    return true;
}

int TrackedEntity::isVisibleTo(EntityTracker* tracker, const boost::shared_ptr<ServerPlayer>& player, bool force,
                               bool skipRangeCheck)
{
    if (mEntity == NULL)
        return 0;

    double xd = player->mX - mXpu;
    double zd = player->mZ - mZpu;
    if (mEntity->mForcedLoading)
    {
        xd = player->mX - mXp / 32;
        zd = player->mZ - mZp / 32;
    }

    int range = mMaxRange;
    if (mRange < mMaxRange)
        range = mRange;
    if (range > 4)
        range -= player->getPlayerViewDistanceModifier();

    bool inRange = false;
    if (!skipRangeCheck && xd >= -range && xd <= range && zd >= -range && zd <= range)
        inRange = true;

    if (!inRange && mEntity->hasPassenger(EntityType::PLAYER))
    {
        std::vector<boost::weak_ptr<Entity> > passengers = *mEntity->getPassengers();
        for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers.begin(); it != passengers.end(); ++it)
        {
            boost::shared_ptr<Entity> passenger = it->lock();
            if (passenger != NULL)
            {
                boost::shared_ptr<TrackedEntity> passengerTracker = tracker->getTracker(passenger);
                if (passengerTracker != NULL)
                {
                    int visible = passengerTracker->isVisibleTo(tracker, player, force, skipRangeCheck);
                    if (visible != 0)
                        return visible;
                }
            }
        }
    }

    bool canSee = canBySeenBy(player);
    MinecraftServer* server = gMinecraftServer;
    if (!inRange)
    {
        INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
        if (networkPlayer != NULL)
        {
            for (unsigned int i = 0; i < server->getPlayers()->mPlayers.size(); ++i)
            {
                boost::shared_ptr<ServerPlayer>& other = server->getPlayers()->mPlayers[i];
                if (other == player || other == mEntity || other->mDimension != player->mDimension)
                    continue;
                INetworkPlayer* otherNetworkPlayer = other->mConnection->getNetworkPlayer();
                if (otherNetworkPlayer == NULL || !networkPlayer->IsSameSystem(otherNetworkPlayer))
                    continue;
                double otherXd = other->mX - mXpu;
                double otherZd = other->mZ - mZpu;
                inRange = inRange | (otherXd >= -range && otherXd <= range && otherZd >= -range && otherZd <= range);
                canSee = canSee | canBySeenBy(other);
            }
        }
    }

    if (force && canSee)
        canSee = mSeenBy.find(player) != mSeenBy.end();

    bool inLevel = false;
    std::vector<boost::shared_ptr<Player> >& levelPlayers = player->mLevel->mPlayers;
    if (canSee)
    {
        std::vector<boost::shared_ptr<Player> >::iterator it = levelPlayers.begin();
        while (it != levelPlayers.end() && *it != player)
            ++it;
        if (it != levelPlayers.end())
            inLevel = true;
    }

    if (inRange)
    {
        if (inLevel)
            return 2;
        return 1;
    }
    return 0;
}

boost::shared_ptr<Packet> TrackedEntity::getAddEntityPacket()
{
    if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, mEntity->GetType()))
    {
        boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(mEntity);
        PlayerUID xuid(0);
        PlayerUID onlineXuid(0);
        if (player != NULL)
        {
            xuid = player->getXuid();
            onlineXuid = player->mOnlineXuid;
        }
        return boost::shared_ptr<Packet>(
            new ClientboundAddPlayerPacket(player, xuid, onlineXuid, mXp, mYp, mZp, mYRotp, mXRotp, mYHeadRotp));
    }

    if (boost::dynamic_pointer_cast<Creature>(mEntity) != NULL
        || eTYPE_DERIVED_FROM(EntityType::SHULKER, mEntity->GetType()))
    {
        mYHeadRotp = Mth::floor(mEntity->getYHeadRot() * 256.0f / 360.0f);
        return boost::shared_ptr<Packet>(new ClientboundAddMobPacket(boost::static_pointer_cast<Mob>(mEntity), mYRotp,
                                                                     mXRotp, mXp, mYp, mZp, mYHeadRotp,
                                                                     mEntity->getAdditionContructionTag()));
    }

    if (eTYPE_DERIVED_FROM(EntityType::PAINTING, mEntity->GetType()))
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddPaintingPacket(boost::static_pointer_cast<Painting>(mEntity)));
    }

    if (eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, mEntity->GetType()))
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 2, 1));

    if (eTYPE_DERIVED_FROM(EntityType::MINECART, mEntity->GetType()))
    {
        boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(mEntity);
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 10, minecart->getType()->mId, mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::BOAT, mEntity->GetType()))
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 1, mYRotp, mXRotp, mXp, mYp, mZp));

    if (eTYPE_DERIVED_FROM(EntityType::EXPERIENCE_ORB, mEntity->GetType()))
    {
        return boost::shared_ptr<Packet>(new ClientboundAddExperienceOrbPacket(
            boost::static_pointer_cast<ExperienceOrb>(mEntity), mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::ENDER_DRAGON, mEntity->GetType()))
    {
        mYHeadRotp = Mth::floor(mEntity->getYHeadRot() * 256.0f / 360.0f);
        return boost::shared_ptr<Packet>(new ClientboundAddMobPacket(boost::static_pointer_cast<LivingEntity>(mEntity),
                                                                     mYRotp, mXRotp, mXp, mYp, mZp, mYHeadRotp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::FISHING_HOOK, mEntity->GetType()))
    {
        boost::shared_ptr<Entity> owner = boost::static_pointer_cast<FishingHook>(mEntity)->getOwner();
        int ownerId = owner != NULL ? owner->getId() : mEntity->getId();
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x5a, ownerId, mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::SPECTRAL_ARROW, mEntity->GetType()))
    {
        boost::shared_ptr<Entity> owner = boost::static_pointer_cast<SpectralArrow>(mEntity)->mOwner;
        int ownerId = owner != NULL ? owner->getId() : mEntity->getId();
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x5b, ownerId + 1, mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::TIPPABLE_ARROW, mEntity->GetType()))
    {
        boost::shared_ptr<Entity> owner = boost::static_pointer_cast<Arrow>(mEntity)->mOwner;
        int ownerId = owner != NULL ? owner->getId() : mEntity->getId();
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x3c, ownerId + 1, mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::THROWN_TRIDENT, mEntity->GetType()))
    {
        boost::shared_ptr<Entity> owner = boost::static_pointer_cast<Arrow>(mEntity)->mOwner;
        int ownerId = owner != NULL ? owner->getId() : mEntity->getId();
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x5e, ownerId + 1, mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::SNOWBALL, mEntity->GetType()))
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 0x3d, mYRotp, mXRotp, mXp, mYp, mZp));

    if (eTYPE_DERIVED_FROM(EntityType::LLAMA_SPIT, mEntity->GetType()))
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x44, 0, mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::THROWN_POTION, mEntity->GetType()))
    {
        boost::shared_ptr<ThrownPotion> thrownPotion = boost::static_pointer_cast<ThrownPotion>(mEntity);
        not_null_ptr<ItemInstance> potion = thrownPotion->getItem();
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 0x49, Item::getId(potion->getItem()),
                                                                        mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (eTYPE_DERIVED_FROM(EntityType::THROWN_EXP_BOTTLE, mEntity->GetType()))
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 0x4b, mYRotp, mXRotp, mXp, mYp, mZp));

    if (eTYPE_DERIVED_FROM(EntityType::THROWN_ENDERPEARL, mEntity->GetType()))
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 0x41, mYRotp, mXRotp, mXp, mYp, mZp));

    if (eTYPE_DERIVED_FROM(EntityType::EYE_OF_ENDER_SIGNAL, mEntity->GetType()))
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 0x48, mYRotp, mXRotp, mXp, mYp, mZp));

    if (eTYPE_DERIVED_FROM(EntityType::FIREWORKS_ROCKET_ENTITY, mEntity->GetType()))
    {
        boost::shared_ptr<LivingEntity> owner = boost::static_pointer_cast<FireworksRocketEntity>(mEntity)->getOwner();
        if (owner == NULL)
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x4c, 0, mYRotp, mXRotp, mXp, mYp, mZp));
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x4c, owner->getId(), mYRotp, mXRotp, mXp, mYp, mZp));
    }

    if (!eTYPE_DERIVED_FROM(EntityType::FIREBALL, mEntity->GetType()))
    {
        if (eTYPE_DERIVED_FROM(EntityType::SHULKER_BULLET, mEntity->GetType()))
        {
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x43, 0, mYRotp, mXRotp, static_cast<int>(mEntity->mXd),
                                               static_cast<int>(mEntity->mYd), static_cast<int>(mEntity->mZd)));
        }
        if (eTYPE_DERIVED_FROM(EntityType::THROWN_EGG, mEntity->GetType()))
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x3e, mYRotp, mXRotp, mXp, mYp, mZp));
        if (eTYPE_DERIVED_FROM(EntityType::EVOCATION_FANGS, mEntity->GetType()))
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x4f, mYRotp, mXRotp, mXp, mYp, mZp));
        if (eTYPE_DERIVED_FROM(EntityType::PRIMED_TNT, mEntity->GetType()))
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x32, mYRotp, mXRotp, mXp, mYp, mZp));
        if (eTYPE_DERIVED_FROM(EntityType::ENDER_CRYSTAL, mEntity->GetType()))
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x33, mYRotp, mXRotp, mXp, mYp, mZp));
        if (eTYPE_DERIVED_FROM(EntityType::FALLING_BLOCK_ENTITY, mEntity->GetType()))
        {
            boost::shared_ptr<FallingBlockEntity> fallingBlock = boost::static_pointer_cast<FallingBlockEntity>(
                mEntity);
            return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(
                mEntity, 0x46, Block::getIdWithData(fallingBlock->getBlock()), mYRotp, mXRotp, mXp, mYp, mZp));
        }
        if (eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, mEntity->GetType()))
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x4e, mYRotp, mXRotp, mXp, mYp, mZp));
        if (eTYPE_DERIVED_FROM(EntityType::ITEM_FRAME, mEntity->GetType()))
        {
            boost::shared_ptr<ItemFrame> frame = boost::static_pointer_cast<ItemFrame>(mEntity);
            return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(
                mEntity, 0x47, frame->mDirection->get2DDataValue(), mYRotp, mXRotp, mXp, mYp, mZp));
        }
        if (eTYPE_DERIVED_FROM(EntityType::LEASH_FENCE_KNOT_ENTITY, mEntity->GetType()))
        {
            boost::shared_ptr<LeashFenceKnotEntity> knot = boost::static_pointer_cast<LeashFenceKnotEntity>(mEntity);
            BlockPos pos = knot->getPos();
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x4d, 0, pos, mYRotp, mXRotp, mXp, mYp, mZp));
        }
        if (eTYPE_DERIVED_FROM(EntityType::AREA_EFFECT_CLOUD, mEntity->GetType()))
        {
            boost::shared_ptr<Entity> owner = boost::static_pointer_cast<AreaEffectCloud>(mEntity)->getOwner();
            int ownerId = owner != NULL ? owner->getId() : mEntity->getId();
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 3, ownerId + 1, mYRotp, mXRotp, mXp, mYp, mZp));
        }
#ifdef MINECRAFT_PC
        if (PcModHooksIsModType(mEntity->GetType()))
        {
            int id = EntityIO::sRegistry.getId(mEntity->GetType());
            if (id >= 0)
                return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, PC_MOD_ENTITY_PACKET, id, mYRotp,
                                                                                mXRotp, mXp, mYp, mZp));
        }
#endif
        return boost::shared_ptr<Packet>();
    }

    unsigned long long fireballType = mEntity->GetType();
    int packetType = 0x3f;
    if (fireballType == EntityType::SMALL_FIREBALL)
        packetType = 0x40;
    else if (fireballType == EntityType::DRAGON_FIREBALL)
        packetType = 0x5d;
    else if (fireballType == EntityType::WITHER_SKULL)
        packetType = 0x42;
    boost::shared_ptr<Fireball> fireball = boost::static_pointer_cast<Fireball>(mEntity);

    boost::shared_ptr<ClientboundAddEntityPacket> packet;
    if (fireball->mOwner == NULL)
    {
        packet = boost::shared_ptr<ClientboundAddEntityPacket>(
            new ClientboundAddEntityPacket(mEntity, packetType, 0, mYRotp, mXRotp, mXp, mYp, mZp));
    }
    else
    {
        packet = boost::shared_ptr<ClientboundAddEntityPacket>(new ClientboundAddEntityPacket(
            mEntity, packetType, fireball->mOwner->getId(), mYRotp, mXRotp, mXp, mYp, mZp));
    }

    double power = fireball->mPowerX;
    if (power < -4.0)
        power = -4.0;
    if (power > 4.0)
        power = 4.0;
    packet->setXa(static_cast<int>(((power + 4.0) / 8.0) * 65535.0 + -32768.0));
    power = fireball->mPowerY;
    if (power < -4.0)
        power = -4.0;
    if (power > 4.0)
        power = 4.0;
    packet->setYa(static_cast<int>(((power + 4.0) / 8.0) * 65535.0 + -32768.0));
    power = fireball->mPowerZ;
    if (power < -4.0)
        power = -4.0;
    if (power > 4.0)
        power = 4.0;
    packet->setZa(static_cast<int>(((power + 4.0) / 8.0) * 65535.0 + -32768.0));
    return packet;
}

bool TrackedEntity::ShouldSendEntityDataOnContruction(boost::shared_ptr<Entity> entity)
{
    const bool isPotion = eTYPE_DERIVED_FROM(EntityType::THROWN_POTION, entity->GetType());
    if (isPotion && Minecraft::GetInstance()->GetMiniGame()->HasFeatureSplashLevitatePotions())
    {
        return false;
    }
    const bool isFireworks = eTYPE_DERIVED_FROM(EntityType::FIREWORKS_ROCKET_ENTITY, entity->GetType());
    if (isFireworks && Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        return false;
    }
    return true;
}

void TrackedEntity::updatePlayer(EntityTracker* tracker, boost::shared_ptr<ServerPlayer> player)
{
    if (player == mEntity)
        return;
#ifdef MINECRAFT_PC
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, mEntity->GetType())
        && boost::static_pointer_cast<Player>(mEntity)->mHiddenHost)
        return;
#endif

    bool canReceiveAll = gMinecraftServer->getPlayers()->canReceiveAllPackets(player);
    int visible = isVisibleTo(tracker, player, false, !canReceiveAll);
    bool notSeen = mSeenBy.find(player) == mSeenBy.end();
    bool forcedLoading = mEntity->mForcedLoading;

    if (visible == 2 && (notSeen | forcedLoading))
    {
        mSeenBy.insert(player);
        boost::shared_ptr<Packet> packet = getAddEntityPacket();
        player->mConnection->send(packet);

        if (Minecraft::InMiniGame(eMiniGameId_Any, true)
            && eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, mEntity->GetType()))
        {
            boost::shared_ptr<ServerPlayer> trackedPlayer = boost::static_pointer_cast<ServerPlayer>(mEntity);
            GameModePacket::SetCameraData cameraData;
            cameraData.camera = trackedPlayer->mPlayerIndex;
            if (trackedPlayer->GetCamera() == NULL)
                cameraData.target = -1;
            else
                cameraData.target = trackedPlayer->GetCamera()->getId();
            player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_SetCamera, cameraData));
        }

        mXap = mEntity->mXd;
        mYap = mEntity->mYd;
        mZap = mEntity->mZd;

        boost::shared_ptr<ClientboundAddMobPacket> addMobPacket = boost::dynamic_pointer_cast<ClientboundAddMobPacket>(
            packet);
        if (!mEntity->getEntityData()->isEmpty() && ShouldSendEntityDataOnContruction(mEntity))
        {
            player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundSetEntityDataPacket(
                mEntity->getId(), static_cast<int>(mEntity->GetType()), mEntity->getEntityData(), true)));
        }

        bool sendMotion = mTrackDelta;
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, mEntity->GetType()))
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(mEntity);
            ModifiableAttributeMap::AttributeInstanceSet* attributes =
                static_cast<ModifiableAttributeMap*>(living->getAttributes())->getSyncableAttributes();
            if (!attributes->empty())
            {
                player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundUpdateAttributesPacket(
                    mEntity->getId(), static_cast<int>(mEntity->GetType()), attributes)));
            }
            delete attributes;
            if (living->isFallFlying())
                sendMotion = true;
        }

        if (sendMotion && addMobPacket == NULL)
        {
            player->mConnection->send(boost::shared_ptr<Packet>(
                new ClientboundSetEntityMotionPacket(mEntity->getId(), mEntity->mXd, mEntity->mYd, mEntity->mZd)));
        }

        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, mEntity->GetType()))
        {
            for (int i = 0; i < 6; ++i)
            {
                const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
                not_null_ptr<ItemInstance> item = boost::static_pointer_cast<LivingEntity>(mEntity)->getItemSlot(slot);
                if (!item->isEmpty())
                {
                    player->mConnection->send(
                        boost::shared_ptr<Packet>(new ClientboundSetEquippedItemPacket(mEntity->getId(), slot, item)));
                }
            }
        }

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, mEntity->GetType()))
        {
            boost::shared_ptr<Player> trackedPlayer = boost::static_pointer_cast<Player>(mEntity);
            if (trackedPlayer->isSleeping())
            {
                player->mConnection->send(
                    boost::shared_ptr<Packet>(new ClientboundPlayerSleepPacket(trackedPlayer, BlockPos(mEntity))));
            }
        }

        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, mEntity->GetType()))
        {
            std::vector<MobEffectInstance*>* effects =
                boost::static_pointer_cast<LivingEntity>(mEntity)->getActiveEffects();
            for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
            {
                player->mConnection->send(
                    boost::shared_ptr<Packet>(new ClientboundUpdateMobEffectPacket(mEntity->getId(), *it)));
            }
            delete effects;
        }

        if (eTYPE_DERIVED_FROM(EntityType::MINECART_TNT, mEntity->GetType()))
        {
            boost::shared_ptr<MinecartTNT> minecart = boost::static_pointer_cast<MinecartTNT>(mEntity);
            if (minecart != NULL && minecart->isPrimed())
                player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundEntityEventPacket(mEntity, 40, 0)));
        }

        if (!mEntity->getPassengers()->empty())
            player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundSetPassengersPacket(mEntity)));

        if (mEntity->isPassenger())
            player->mConnection->send(
                boost::shared_ptr<Packet>(new ClientboundSetPassengersPacket(mEntity->getVehicle())));

        if (eTYPE_DERIVED_FROM(EntityType::MOB, mEntity->GetType()))
        {
            boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(mEntity);
            if (mob->isLeashed())
            {
                player->mConnection->send(
                    boost::shared_ptr<Packet>(new ClientboundSetEntityLinkPacket(2, mob, mob->getLeashHolder())));
            }
        }

        mEntity->startSeenByPlayer(player);
        player->cancelRemoveEntity(mEntity);
        return;
    }

    if (visible != 0)
        return;

    boost::unordered_set<boost::shared_ptr<ServerPlayer>, _PlayerKeyHash, _PlayerKeyEq>::iterator it = mSeenBy.find(
        player);
    if (it == mSeenBy.end())
        return;
    mSeenBy.erase(it);
    mEntity->stopSeenByPlayer(player);
    player->sendRemoveEntity(mEntity);
}

void TrackedEntity::updatePlayers(EntityTracker* tracker, std::vector<boost::shared_ptr<Player> >* players)
{
    for (unsigned int i = 0; i < players->size(); ++i)
        updatePlayer(tracker, boost::dynamic_pointer_cast<ServerPlayer>(players->at(i)));
}

void TrackedEntity::broadcast(boost::shared_ptr<Packet> packet)
{
    if (Packet::canSendToAnyClient(packet))
    {
        std::vector<boost::shared_ptr<ServerPlayer> > sentTo;
        for (boost::unordered_set<boost::shared_ptr<ServerPlayer>, _PlayerKeyHash, _PlayerKeyEq>::iterator it =
                 mSeenBy.begin();
             it != mSeenBy.end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> player = *it;
            bool dontSend = false;
            if (!sentTo.empty())
            {
                INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
                if (networkPlayer == NULL)
                {
                    dontSend = true;
                }
                else
                {
                    for (unsigned int i = 0; i < sentTo.size(); ++i)
                    {
                        boost::shared_ptr<ServerPlayer> sent = sentTo[i];
                        INetworkPlayer* sentNetworkPlayer = sent->mConnection->getNetworkPlayer();
                        if (sentNetworkPlayer != NULL && networkPlayer->IsSameSystem(sentNetworkPlayer))
                            dontSend = true;
                    }
                }
            }
            if (dontSend)
                continue;
            (*it)->mConnection->send(packet);
            sentTo.push_back(player);
        }
    }
    else
    {
        for (boost::unordered_set<boost::shared_ptr<ServerPlayer>, _PlayerKeyHash, _PlayerKeyEq>::iterator it =
                 mSeenBy.begin();
             it != mSeenBy.end(); ++it)
        {
            (*it)->mConnection->send(packet);
        }
    }
}

void TrackedEntity::broadcastAndSend(boost::shared_ptr<Packet> packet)
{
    std::vector<boost::shared_ptr<ServerPlayer> > sentTo;
    broadcast(packet);
    const bool isServerPlayer = eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, mEntity->GetType());
    boost::shared_ptr<ServerPlayer> player = isServerPlayer ? boost::static_pointer_cast<ServerPlayer>(mEntity)
                                                            : boost::shared_ptr<ServerPlayer>();
    if (player != NULL && player->mConnection != NULL)
        player->mConnection->send(packet);
}

void TrackedEntity::sendDirtyEntityData()
{
    boost::shared_ptr<SynchedEntityData> entityData = mEntity->getEntityData();
    if (entityData->isDirty())
    {
        broadcastAndSend(boost::shared_ptr<Packet>(new ClientboundSetEntityDataPacket(
            mEntity->getId(), static_cast<int>(mEntity->GetType()), entityData, false)));
    }

    const bool isLiving = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, mEntity->GetType());
    if (isLiving)
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(mEntity);
        ModifiableAttributeMap::AttributeInstanceSet* attributes =
            static_cast<ModifiableAttributeMap*>(living->getAttributes())->getDirtyAttributes();
        if (!attributes->empty())
        {
            broadcastAndSend(boost::shared_ptr<Packet>(new ClientboundUpdateAttributesPacket(
                mEntity->getId(), static_cast<int>(mEntity->GetType()), attributes)));
        }
        attributes->clear();
    }
}

void TrackedEntity::resetPlayerVisibility()
{
    mUpdatedPlayerVisibility = false;
}

void TrackedEntity::tick(EntityTracker* tracker, std::vector<boost::shared_ptr<Player> >* players)
{
    mPlayerEntitiesUpdated = false;
    if (!mUpdatedPlayerVisibility || mEntity->distanceToSqr(mXpu, mYpu, mZpu) > 16.0)
    {
        mXpu = mEntity->mX;
        mYpu = mEntity->mY;
        mZpu = mEntity->mZ;
        mUpdatedPlayerVisibility = true;
        mPlayerEntitiesUpdated = true;
        updatePlayers(tracker, players);
    }

    std::vector<boost::weak_ptr<Entity> >* passengers = mEntity->getPassengers();
    bool passengersChanged = passengers->size() != mLastPassengers.size();
    for (std::vector<boost::weak_ptr<Entity> >::iterator passenger = passengers->begin();
         !passengersChanged && passenger != passengers->end(); ++passenger)
    {
        bool found = false;
        for (std::vector<boost::weak_ptr<Entity> >::iterator last = mLastPassengers.begin();
             last != mLastPassengers.end(); ++last)
        {
            if (last->lock() == passenger->lock())
            {
                found = true;
                break;
            }
        }
        if (!found)
            passengersChanged = true;
    }
    if (passengersChanged)
    {
        mLastPassengers.clear();
        mLastPassengers = *passengers;
        broadcast(boost::shared_ptr<Packet>(new ClientboundSetPassengersPacket(mEntity)));
    }

    if (eTYPE_DERIVED_FROM(EntityType::ITEM_FRAME, mEntity->GetType()) && mTickCount % 10 == 0)
    {
        boost::shared_ptr<ItemFrame> frame = boost::static_pointer_cast<ItemFrame>(mEntity);
        not_null_ptr<ItemInstance> item = frame->getItem();
        if (item->getItem() == Items::filledMap && !mEntity->mRemoved)
        {
            boost::shared_ptr<MapItemSavedData> savedData =
                static_cast<MapItem*>(Items::filledMap)->getSavedData(item, mEntity->mLevel);
            for (std::vector<boost::shared_ptr<Player> >::iterator it = players->begin(); it != players->end(); ++it)
            {
                boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(*it);
                savedData->tickCarriedBy(player, item);
                if (!player->mRemoved && player->mConnection != NULL && player->mConnection->countDelayedPackets() <= 5)
                {
                    boost::shared_ptr<Packet> packet =
                        static_cast<MapItem*>(Items::filledMap)->getUpdatePacket(item, mEntity->mLevel, player);
                    if (packet != NULL)
                        player->mConnection->send(packet);
                }
            }
        }

        boost::shared_ptr<SynchedEntityData> entityData = mEntity->getEntityData();
        if (entityData->isDirty())
        {
            broadcastAndSend(boost::shared_ptr<Packet>(new ClientboundSetEntityDataPacket(
                mEntity->getId(), static_cast<int>(mEntity->GetType()), entityData, false)));
        }
    }

    if (mTickCount % mUpdateInterval == 0 || mEntity->mHasImpulse || mEntity->getEntityData()->isDirty())
    {
        if (mEntity->isPassenger())
        {
            int yRot = Mth::floor(mEntity->mYRot * 256.0f / 360.0f);
            int xRot = Mth::floor(mEntity->mXRot * 256.0f / 360.0f);
            if (std::abs(yRot - mYRotp) >= 1 || std::abs(xRot - mXRotp) >= 1)
            {
                broadcast(boost::shared_ptr<Packet>(
                    new ClientboundMoveEntityPacket::Rot(mEntity->getId(), static_cast<unsigned char>(yRot),
                                                         static_cast<unsigned char>(xRot), mEntity->mOnGround)));
                mYRotp = yRot;
                mXRotp = xRot;
            }
            mXp = EntityTracker::truncate(mEntity->mX);
            mYp = EntityTracker::truncate(mEntity->mY);
            mZp = EntityTracker::truncate(mEntity->mZ);
            sendDirtyEntityData();
            mWasRiding = true;
        }
        else
        {
            ++mTeleportDelay;
            int x = EntityTracker::truncate(mEntity->mX);
            int y = EntityTracker::truncate(mEntity->mY);
            int z = EntityTracker::truncate(mEntity->mZ);
            int yRot = Mth::floor(Mth::wrapDegrees(mEntity->mYRot) * 256.0f / 360.0f);
            int xRot = Mth::floor(Mth::wrapDegrees(mEntity->mXRot) * 256.0f / 360.0f);
            int xa = x - mXp;
            int ya = y - mYp;
            int za = z - mZp;
            boost::shared_ptr<Packet> packet;
            bool pos = xa * xa + ya * ya + za * za >= 4 || mTickCount % 60 == 0;
            bool rot = std::abs(yRot - mYRotp) >= 1 || std::abs(xRot - mXRotp) >= 1;

            if (mTickCount > 0 || eTYPE_DERIVED_FROM(EntityType::ARROW, mEntity->GetType())
                || eTYPE_DERIVED_FROM(EntityType::PLAYER, mEntity->GetType()))
            {
                if (xa >= -512 && xa < 512 && ya >= -512 && ya < 512 && za >= -512 && za < 512 && !mWasRiding
                    && mWasOnGround == mEntity->mOnGround)
                {
                    if (eTYPE_DERIVED_FROM(EntityType::ARROW, mEntity->GetType()))
                    {
                        rot = true;
                        pos = pos || mTickCount > 0;
                    }

                    bool small = xa >= -16 && xa < 16 && za >= -16 && za < 16 && ya >= -32 && ya < 32;
#ifdef MINECRAFT_PC
                    const bool smallId = static_cast<unsigned int>(mEntity->getId()) < 0x800;
                    small = small && smallId;
#endif
                    if (pos && rot)
                    {
                        if (small)
                        {
                            packet = boost::shared_ptr<Packet>(new MoveEntityPacketSmall::PosRot(
                                mEntity->getId(), static_cast<signed char>(xa), static_cast<signed char>(ya),
                                static_cast<signed char>(za), static_cast<signed char>(yRot),
                                static_cast<signed char>(xRot), mEntity->mOnGround));
                            ++sSmallPosRotPacketCount;
                        }
                        else
                        {
                            packet = boost::shared_ptr<Packet>(new ClientboundMoveEntityPacket::PosRot(
                                mEntity->getId(), xa, ya, za, static_cast<unsigned char>(yRot),
                                static_cast<unsigned char>(xRot), mEntity->mOnGround));
                            ++sPosRotPacketCount;
                        }
                    }
                    else if (pos)
                    {
                        if (small)
                        {
                            packet = boost::shared_ptr<Packet>(new MoveEntityPacketSmall::Pos(
                                mEntity->getId(), static_cast<signed char>(xa), static_cast<signed char>(ya),
                                static_cast<signed char>(za), mEntity->mOnGround));
                            ++sSmallPosPacketCount;
                        }
                        else
                        {
                            packet = boost::shared_ptr<Packet>(
                                new ClientboundMoveEntityPacket::Pos(mEntity->getId(), xa, ya, za, mEntity->mOnGround));
                            ++sPosPacketCount;
                        }
                    }
                    else if (rot)
                    {
#ifdef MINECRAFT_PC
                        if (xRot == mXRotp && smallId)
#else
                        if (xRot == mXRotp)
#endif
                        {
                            packet = boost::shared_ptr<Packet>(
                                new MoveEntityPacketSmall::Rot(mEntity->getId(), static_cast<signed char>(yRot),
                                                               static_cast<signed char>(xRot), mEntity->mOnGround));
                            ++sSmallRotPacketCount;
                        }
                        else
                        {
                            packet = boost::shared_ptr<Packet>(new ClientboundMoveEntityPacket::Rot(
                                mEntity->getId(), static_cast<unsigned char>(yRot), static_cast<unsigned char>(xRot),
                                mEntity->mOnGround));
                            ++sRotPacketCount;
                        }
                    }
                }
                else
                {
                    mWasOnGround = mEntity->mOnGround;
                    mTeleportDelay = 0;
                    resetPlayerVisibility();
                    packet = boost::shared_ptr<Packet>(new ClientboundTeleportEntityPacket(mEntity));
                    mXp = x;
                    mYp = y;
                    mZp = z;
                    mYRotp = yRot;
                    mXRotp = xRot;
                }
            }
            else
            {
                pos = false;
                rot = false;
            }

            bool fallFlying = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, mEntity->GetType())
                              && boost::static_pointer_cast<LivingEntity>(mEntity)->isFallFlying();
            if ((fallFlying || mTrackDelta) && mTickCount > 0)
            {
                double xd = mEntity->mXd - mXap;
                double yd = mEntity->mYd - mYap;
                double zd = mEntity->mZd - mZap;
                double distance = xd * xd + yd * yd + zd * zd;
                if (distance > 0.0004
                    || (distance > 0.0 && mEntity->mXd == 0.0 && mEntity->mYd == 0.0 && mEntity->mZd == 0.0))
                {
                    mXap = mEntity->mXd;
                    mYap = mEntity->mYd;
                    mZap = mEntity->mZd;
                    broadcast(boost::shared_ptr<Packet>(
                        new ClientboundSetEntityMotionPacket(mEntity->getId(), mXap, mYap, mZap)));
                }
            }

            if (packet != NULL)
                broadcast(packet);
            sendDirtyEntityData();
            if (pos)
            {
                mXp = x;
                mYp = y;
                mZp = z;
            }
            if (rot)
            {
                mYRotp = yRot;
                mXRotp = xRot;
            }
            mWasRiding = false;
        }

        int yHeadRot = Mth::floor(mEntity->getYHeadRot() * 256.0f / 360.0f);
        if (std::abs(yHeadRot - mYHeadRotp) >= 1)
        {
            broadcast(boost::shared_ptr<Packet>(
                new ClientboundRotateHeadPacket(mEntity, static_cast<unsigned char>(yHeadRot))));
            mYHeadRotp = yHeadRot;
        }
        mEntity->mHasImpulse = false;
        ++mTickCount;
    }
    else
    {
        ++mTickCount;
    }

    if (mEntity->mHurtMarked)
    {
        broadcastAndSend(boost::shared_ptr<Packet>(new ClientboundSetEntityMotionPacket(mEntity)));
        mEntity->mHurtMarked = false;
    }
}

void TrackedEntity::broadcastRemoved()
{
    for (boost::unordered_set<boost::shared_ptr<ServerPlayer>, _PlayerKeyHash, _PlayerKeyEq>::iterator it =
             mSeenBy.begin();
         it != mSeenBy.end(); ++it)
    {
        mEntity->stopSeenByPlayer(*it);
        (*it)->sendRemoveEntity(mEntity);
    }
}

void TrackedEntity::removePlayer(EntityTracker* tracker, boost::shared_ptr<ServerPlayer> player)
{
    boost::unordered_set<boost::shared_ptr<ServerPlayer>, _PlayerKeyHash, _PlayerKeyEq>::iterator it = mSeenBy.find(
        player);
    if (it == mSeenBy.end())
        return;
    mSeenBy.erase(it);
    if (isVisibleTo(tracker, player, false, true) == 0)
    {
        mEntity->stopSeenByPlayer(player);
        player->mEntitiesToRemove.push_back(mEntity->getId());
    }
}

void TrackedEntity::clear(boost::shared_ptr<ServerPlayer> player)
{
    boost::unordered_set<boost::shared_ptr<ServerPlayer>, _PlayerKeyHash, _PlayerKeyEq>::iterator it = mSeenBy.find(
        player);
    if (it == mSeenBy.end())
        return;
    mSeenBy.erase(it);
    mEntity->stopSeenByPlayer(player);
    player->sendRemoveEntity(mEntity);
}

boost::shared_ptr<Entity> TrackedEntity::getEntity()
{
    return mEntity;
}

void TrackedEntity::setMaxRange(int maxRange)
{
    mMaxRange = maxRange;
}
