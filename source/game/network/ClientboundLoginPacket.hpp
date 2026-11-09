#ifndef GAME_NETWORK_CLIENTBOUNDLOGINPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDLOGINPACKET_HPP

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/generation/PlayerUID.hpp"

class DataInputStream;
class DataOutputStream;
class LevelType;
class PacketListener;
class ServerConnectionListener;

class ClientboundLoginPacket : public Packet, public boost::enable_shared_from_this<ClientboundLoginPacket>
{
    friend class ClientPacketListener;
    friend class ServerConnectionListener;
    friend class PlayerList;

public:
    ClientboundLoginPacket();
    ClientboundLoginPacket(const std::wstring&, int, LevelType*, long long, int, int, int, char, unsigned char,
                           unsigned char, char, int, unsigned char, bool, unsigned int, int, int, int, bool,
                           unsigned char, int, int, int);
    ClientboundLoginPacket(const std::wstring&, int, PlayerUID, PlayerUID, bool, unsigned int, unsigned int,
                           unsigned int, bool, bool);
    virtual int getPacketId();
    virtual void read(DataInputStream*);
    virtual void write(DataOutputStream*);
    virtual void handle(PacketListener*);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundLoginPacket()); }

private:
    int mEntityId;
    std::wstring mLevelName;
    long long mSeed;
    char mGameMode;
    PlayerUID mPlayerUID;
    PlayerUID mPlayerUID2;
    char mPlayerFlag;
    bool mHardcore;
    int mViewDistance;
    int mChunkDistance;
    unsigned char mReducedDebugInfo;
    unsigned int mWorldHeight;
    unsigned int mSeaLevel;
    bool mAllowCommands;
    bool mIsFlat;
    LevelType* mLevelType;
    int mCurrentTick;
    int mXZSize;
    int mHellScale;
    int mMaxTick;
    bool mIsFlatWorld;
    unsigned char mPlayerFlags;
    int mSpawnProtection;
    int mSimulationDistance;
    bool mHasAchievements;
    int mUnknownValue3;
    int mGameType;
    unsigned char mDifficultyValue;
    unsigned char mMaxPlayers;
    int mDimension;
    int mDifficulty;
};

#endif
