#ifndef GAME_NETWORK_CLIENTBOUNDCHATPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCHATPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/FlexiblePacket.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class Level;
class Packet;
class PacketListener;
class ClientPacketListener;

class ClientboundChatPacket : public FlexiblePacket, public boost::enable_shared_from_this<ClientboundChatPacket>
{
public:
    enum EChatPacketMessage
    {
        eChatPacketMessage_Text = 0,
        eChatPacketMessage_BedOccupied = 1,
        eChatPacketMessage_BedNoSleep = 2,
        eChatPacketMessage_BedNotValid = 3,
        eChatPacketMessage_BedNotSafe = 4,
        eChatPacketMessage_BedTooFarAway = 5,
        eChatPacketMessage_BedPlayerSleep = 6,
        eChatPacketMessage_BedMeSleep = 7,
        eChatPacketMessage_PlayerLeft = 8,
        eChatPacketMessage_PlayerJoined = 9,
        eChatPacketMessage_PlayerKicked = 10,
        eChatPacketMessage_PlayerVotedToKick = 11,
        eChatPacketMessage_InteractionNotAllowed = 12,
        eChatPacketMessage_DeathInFire = 13,
        eChatPacketMessage_DeathLightningBolt = 14,
        eChatPacketMessage_DeathOnFire = 15,
        eChatPacketMessage_DeathLava = 16,
        eChatPacketMessage_DeathInWall = 17,
        eChatPacketMessage_DeathCramming = 18,
        eChatPacketMessage_DeathDrown = 19,
        eChatPacketMessage_DeathStarve = 20,
        eChatPacketMessage_DeathCactus = 21,
        eChatPacketMessage_DeathFall = 22,
        eChatPacketMessage_DeathFlyIntoWall = 23,
        eChatPacketMessage_DeathOutOfWorld = 24,
        eChatPacketMessage_DeathCustom = 25,
        eChatPacketMessage_DeathGeneric = 26,
        eChatPacketMessage_DeathExplosion = 27,
        eChatPacketMessage_DeathMagic = 28,
        eChatPacketMessage_DeathMob = 29,
        eChatPacketMessage_DeathPlayer = 30,
        eChatPacketMessage_DeathArrow = 31,
        eChatPacketMessage_DeathFireball = 32,
        eChatPacketMessage_DeathThrown = 33,
        eChatPacketMessage_DeathIndirectMagic = 34,
        eChatPacketMessage_DeathDragonBreath = 35,
        eChatPacketMessage_DeathAnvil = 36,
        eChatPacketMessage_DeathFallingBlock = 37,
        eChatPacketMessage_DeathThorns = 39,
        eChatPacketMessage_DeathExplosionAlternate = 40,
        eChatPacketMessage_DeathDryOut = 41,
        eChatPacketMessage_DeathTrident = 42,
        eChatPacketMessage_DeathFellLadder = 43,
        eChatPacketMessage_DeathFellVines = 44,
        eChatPacketMessage_DeathFellWater = 45,
        eChatPacketMessage_DeathFellHigh = 46,
        eChatPacketMessage_DeathFellDoomed = 47,
        eChatPacketMessage_DeathFellDoomedBy = 48,
        eChatPacketMessage_DeathFellDoomedByItem = 49,
        eChatPacketMessage_DeathFellFinished = 50,
        eChatPacketMessage_DeathFellFinishedItem = 51,
        eChatPacketMessage_DeathInFireFighting = 52,
        eChatPacketMessage_DeathLightningBoltAlternate = 53,
        eChatPacketMessage_DeathOnFireFighting = 54,
        eChatPacketMessage_DeathLavaEscaping = 55,
        eChatPacketMessage_DeathDrownEscaping = 56,
        eChatPacketMessage_DeathCactusEscaping = 57,
        eChatPacketMessage_DeathExplosionBy = 58,
        eChatPacketMessage_DeathWither = 59,
        eChatPacketMessage_DeathMobItem = 60,
        eChatPacketMessage_DeathArrowItem = 61,
        eChatPacketMessage_DeathFireballItem = 62,
        eChatPacketMessage_DeathThrownItem = 63,
        eChatPacketMessage_DeathHotFloor = 64,
        eChatPacketMessage_DeathHotFloorEscaping = 65,
        eChatPacketMessage_DeathTridentItem = 66,
        eChatPacketMessage_DeathIndirectMagicItem = 67,
        eChatPacketMessage_PlayerEnteredEnd = 68,
        eChatPacketMessage_PlayerLeftEnd = 69,
        eChatPacketMessage_MaxAnimals = 70,
        eChatPacketMessage_MaxChickens = 71,
        eChatPacketMessage_MaxSquid = 72,
        eChatPacketMessage_MaxMooshrooms = 73,
        eChatPacketMessage_MaxWolves = 74,
        eChatPacketMessage_MaxEnemies = 76,
        eChatPacketMessage_MaxVillagers = 77,
        eChatPacketMessage_MaxHangingEntities = 78,
        eChatPacketMessage_MaxArmorStands = 79,
        eChatPacketMessage_CantSpawnInPeaceful = 80,
        eChatPacketMessage_MaxBredAnimals = 82,
        eChatPacketMessage_MaxBredChickens = 83,
        eChatPacketMessage_MaxBredMooshrooms = 84,
        eChatPacketMessage_MaxBredWolves = 85,
        eChatPacketMessage_CantShearMooshroom = 86,
        eChatPacketMessage_MaxBoats = 87,
        eChatPacketMessage_MaxMinecarts = 88,
        eChatPacketMessage_MaxBats = 89,
        eChatPacketMessage_MaxBanners = 90,
        eChatPacketMessage_MaxEnderCrystals = 91,
        eChatPacketMessage_MaxBeacons = 92,
        eChatPacketMessage_MaxDolphins = 93,
        eChatPacketMessage_MaxPhantoms = 94,
        eChatPacketMessage_MaxFishAndSquid = 95,
        eChatPacketMessage_TeleportSuccess = 96,
        eChatPacketMessage_TeleportedYouToPosition = 97,
        eChatPacketMessage_TeleportedToYou = 98,
        eChatPacketMessage_TimeSuccess = 99,
        eChatPacketMessage_DifficultySuccess = 100,
        eChatPacketMessage_DefaultGameModeSuccess = 101,
        eChatPacketMessage_DefaultSpawnpointSuccess = 102,
        eChatPacketMessage_DefaultSpawnpointNotValid = 103,
        eChatPacketMessage_SpawnpointSuccess = 104,
        eChatPacketMessage_SpawnpointNotValid = 105,
        eChatPacketMessage_WeatherClear = 106,
        eChatPacketMessage_WeatherRain = 107,
        eChatPacketMessage_WeatherThunder = 108,
        eChatPacketMessage_GameModeSuccessSelf = 109,
        eChatPacketMessage_GameModeSuccessOther = 110,
        eChatPacketMessage_GameModeChanged = 111,
        eChatPacketMessage_CommandFeedback = 112,
        eChatPacketMessage_SelectingMode = 113,
        eChatPacketMessage_WaitingForLoad = 114,
        eChatPacketMessage_RoundChanging = 115,
        eChatPacketMessage_RoundStarting = 116,
        eChatPacketMessage_RoundStart = 117,
        eChatPacketMessage_RoundEnding = 118,
        eChatPacketMessage_RoundEnd = 119,
        eChatPacketMessage_Spectating = 120,
        eChatPacketMessage_LivesRemaining = 121,
        eChatPacketMessage_SecondsToReachEnd = 122,
        eChatPacketMessage_Checkpoint = 123,
        eChatPacketMessage_InvulnerabilityOff = 124,
        eChatPacketMessage_InitializingCountdown = 125,
        eChatPacketMessage_TimeToStartCountdown = 126,
        eChatPacketMessage_TimeToRoundEndCountdown = 127,
        eChatPacketMessage_TimeToGameChangeCountdown = 128,
        eChatPacketMessage_InvulnerabilityCountdown = 129,
        eChatPacketMessage_WaitingForLessThan = 130,
        eChatPacketMessage_WaitingForNoMoreThan = 131,
        eChatPacketMessage_WaitingForMoreThan = 132,
        eChatPacketMessage_WaitingForAtLeast = 133,
        eChatPacketMessage_RoundEndIn = 134,
        eChatPacketMessage_MorePlayersRequired = 135,
        eChatPacketMessage_WaitingForPlayersReady = 136,
        eChatPacketMessage_ItemsSpawnedInArena = 137,
        eChatPacketMessage_ItemsSpawnedAtCentre = 138,
        eChatPacketMessage_SpecialItemsSpawned = 139,
        eChatPacketMessage_TeamsDisabled = 140,
        eChatPacketMessage_MaxTeamSize = 141,
        eChatPacketMessage_NotEnoughSurvivorsToTeam = 142,
        eChatPacketMessage_TeamInvite = 143,
        eChatPacketMessage_TeamInviteExpired = 144,
        eChatPacketMessage_PlayerAddedToTeam = 145,
        eChatPacketMessage_AddedToTeam = 146,
        eChatPacketMessage_PlayerLeavingTeam = 147,
        eChatPacketMessage_LeavingTeam = 148,
        eChatPacketMessage_LeftTeam = 149,
        eChatPacketMessage_TeamsDissolving = 150,
        eChatPacketMessage_TeamsDissolvingCountdown = 151,
        eChatPacketMessage_Fight = 152,
        eChatPacketMessage_ShowdownStarted = 153,
        eChatPacketMessage_NowSpectating = 154,
        eChatPacketMessage_NoPlayersToSpectate = 155,
        eChatPacketMessage_ContainerLocked = 156
    };

    ClientboundChatPacket();
    ClientboundChatPacket(EChatPacketMessage message, int entityType);
    ClientboundChatPacket(EChatPacketMessage message, int* intParams, unsigned int numIntParams,
                          std::wstring* stringParams, unsigned int numStringParams);
    ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, int entityType);
    ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, boost::shared_ptr<Entity> entity,
                          const std::wstring& entityName);
    ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, boost::shared_ptr<Entity> entity,
                          const std::wstring& entityName, const std::wstring& killerName);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    std::wstring getMobCustomName(Level* level);

    EChatPacketMessage getMessage() const { return message; }

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundChatPacket()); }

private:
    friend class ClientPacketListener;
    friend class ClientMasterGameMode;
    friend class CombatTracker;
    friend class GameStats;
    friend class MasterGameMode;
    friend class ServerPlayer;
    EChatPacketMessage message;
};

#endif
