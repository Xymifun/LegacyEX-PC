#ifndef GAME_NETWORK_GAMEMODEPACKET_HPP
#define GAME_NETWORK_GAMEMODEPACKET_HPP

#include <string>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/FlexiblePacket.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class TeamInvite;

enum EGameModeState
{
    eGameModeState_None,
    eGameModeState_LobbyCountdown,
    eGameModeState_Lobby,
    eGameModeState_SelectingGameRules,
    eGameModeState_LoadingRound,
    eGameModeState_WaitingToStart,
    eGameModeState_RoundStarting,
    eGameModeState_RoundPlaying,
    eGameModeState_PostRound,
    eGameModeState_RoundEnding,
    eGameModeState_LoadingSubRound
};

class GameModePacket : public FlexiblePacket, public boost::enable_shared_from_this<GameModePacket>
{
public:
    enum EMessage
    {
        eMessage_Follow = 0,
        eMessage_StartSpectating = 1,
        eMessage_SetCamera = 2,
        eMessage_TransitionToRound = 3,
        eMessage_Transition = 4,
        eMessage_WaitForChunkLoading = 5,
        eMessage_FinishedLoadingLocally = 6,
        eMessage_RoundStart = 7,
        eMessage_SendTeamInvite = 8,
        eMessage_SyncTeamInvite = 9,
        eMessage_RemoveTeamInvite = 10,
        eMessage_AcceptTeamInvite = 11,
        eMessage_Unnamed12 = 12,
        eMessage_AddAreaBounds = 13,
        eMessage_SetAreaBounds = 14,
        eMessage_ClearAreaBounds = 15,
        eMessage_LeaveTeam = 16,
        eMessage_ChangeCustomGameMode = 17,
        eMessage_SetNextCustomGameMode = 18,
        eMessage_LockPlayerLocation = 19,
        eMessage_ChestRefill = 20,
        eMessage_Tutorial = 21,
        eMessage_Showdown = 22,
        eMessage_StateChange = 23,
        eMessage_DeathPosition = 24,
        eMessage_UpdateSeed = 25,
        eMessage_UpdateTheme = 26,
        eMessage_UpdateThemeId = 27,
        eMessage_UpdateStartTime = 28
    };

    class Data
    {
    public:
        virtual ~Data() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet) = 0;
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const = 0;
    };

    class EmptyData : public Data
    {
    public:
        virtual ~EmptyData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;
    };

    class IntData : public Data
    {
    public:
        IntData();
        IntData(int value);
        virtual ~IntData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int value;
    };

    class FollowData : public Data
    {
    public:
        FollowData();
        FollowData(bool follow, bool teleport);
        virtual ~FollowData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        bool follow;
        bool teleport;
    };

    class TransitionData : public Data
    {
    public:
        TransitionData();
        TransitionData(int transition, int level, int x, int y);
        virtual ~TransitionData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int transition;
        int level;
        int x;
        int y;
    };

    class TransitionToRoundData : public TransitionData
    {
    public:
        TransitionToRoundData();
        TransitionToRoundData(int round, int seed, int mapSize, int transition, int level, int x, int y);
        virtual ~TransitionToRoundData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int round;
        int seed;
        int mapSize;
    };

    class ChangeCustomGameMode : public Data
    {
    public:
        ChangeCustomGameMode();
        ChangeCustomGameMode(int gameMode, int customGameMode);
        virtual ~ChangeCustomGameMode() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int gameMode;
        int customGameMode;
    };

    class UpdateStartTimeData : public Data
    {
    public:
        UpdateStartTimeData();
        UpdateStartTimeData(long long startTime);
        virtual ~UpdateStartTimeData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        long long startTime;
    };

    class LockPlayerLocation : public Data
    {
    public:
        LockPlayerLocation();
        LockPlayerLocation(bool locked);
        virtual ~LockPlayerLocation() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        bool locked;
    };

    class ChestRefill : public Data
    {
    public:
        ChestRefill();
        ChestRefill(std::vector<BlockPos> positions);
        virtual ~ChestRefill() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        std::vector<BlockPos> positions;
    };

    class PlayerData : public Data
    {
    public:
        PlayerData();
        PlayerData(std::wstring playerName);
        virtual ~PlayerData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        std::wstring playerName;
    };

    class SendTeamInviteData : public PlayerData
    {
    public:
        virtual ~SendTeamInviteData() {}
    };

    class SetCameraData : public Data
    {
    public:
        virtual ~SetCameraData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int camera;
        int target;
    };

    class SyncTeamInviteData : public Data
    {
    public:
        SyncTeamInviteData();
        SyncTeamInviteData(TeamInvite* invite);
        virtual ~SyncTeamInviteData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        TeamInvite* invite;
    };

    class TeamIDData : public Data
    {
    public:
        TeamIDData();
        TeamIDData(int teamId);
        virtual ~TeamIDData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int teamId;
    };

    class AcceptTeamInviteData : public TeamIDData
    {
    public:
        virtual ~AcceptTeamInviteData();
    };

    class RemoveTeamInviteData : public TeamIDData
    {
    public:
        virtual ~RemoveTeamInviteData();
    };

    class AddAreaBoundsData : public Data
    {
    public:
        AddAreaBoundsData();
        AddAreaBoundsData(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
        virtual ~AddAreaBoundsData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int minX;
        int minY;
        int minZ;
        int maxX;
        int maxY;
        int maxZ;
    };

    class StateChangeData : public Data
    {
    public:
        StateChangeData();
        StateChangeData(EGameModeState state);
        virtual ~StateChangeData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        EGameModeState state;
    };

    class TutorialGameModeData : public Data
    {
    public:
        TutorialGameModeData();
        TutorialGameModeData(int tutorial);
        virtual ~TutorialGameModeData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int tutorial;
    };

    class UpdateSeedGameModeData : public Data
    {
    public:
        UpdateSeedGameModeData();
        UpdateSeedGameModeData(long long seed, float mapSizeMultiplier);
        virtual ~UpdateSeedGameModeData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;
        long long getSeed() const;
        double getMapSizeMultiplier() const;

        long long seed;
        float mapSizeMultiplier;
    };

    class UpdateThemeGameModeData : public Data
    {
    public:
        UpdateThemeGameModeData();
        UpdateThemeGameModeData(int themeData);
        virtual ~UpdateThemeGameModeData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;
        unsigned int getThemeData() const;

        int themeData;
    };

    class SoundPositionData : public Data
    {
    public:
        SoundPositionData();
        SoundPositionData(float x, float y, float z);
        virtual ~SoundPositionData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        float x;
        float y;
        float z;
    };

    class UpdateThemeIdData : public Data
    {
    public:
        UpdateThemeIdData();
        UpdateThemeIdData(int themeId, int previousThemeId);
        virtual ~UpdateThemeIdData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;

        int themeId;
        int previousThemeId;
    };

    class RoundStartGameModeData : public Data
    {
    public:
        RoundStartGameModeData();
        RoundStartGameModeData(const std::wstring& roundId, const std::wstring& subRoundId);
        virtual ~RoundStartGameModeData() {}
        virtual void ReadPacket(boost::shared_ptr<GameModePacket> packet);
        virtual void WritePacket(boost::shared_ptr<GameModePacket> packet) const;
        std::wstring GetRoundId() const;
        std::wstring GetSubRoundId() const;

        std::wstring roundId;
        std::wstring subRoundId;
    };

    GameModePacket();
    GameModePacket(EMessage message);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    void WritePacket(const Data& data);
    static boost::shared_ptr<GameModePacket> Create(EMessage message, const Data& data);
    void ReadPacket(Data& data);
    unsigned int GetMessage();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new GameModePacket()); }

private:
    EMessage message;
};

#endif
