#ifndef GAME_NETWORK_CLIENTBOUNDPRELOGINPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDPRELOGINPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/minigame/EMiniGameId.hpp"
#include "game/network/Packet.hpp"

class CPlatformNetworkManagerNintendo;
class DataInputStream;
class DataOutputStream;
class FakeClientPacketListener;
class PacketListener;
struct PlayerUID;

class ClientboundPreLoginPacket : public Packet, public boost::enable_shared_from_this<ClientboundPreLoginPacket>
{
public:
    ClientboundPreLoginPacket();
    ClientboundPreLoginPacket(std::wstring name, PlayerUID* players, unsigned int playerCount, unsigned char gameMode,
                              unsigned int seed, char* uniqueMapName, unsigned int hostPlayer, unsigned int maxPlayers,
                              unsigned char difficulty, unsigned int gameType, bool hardcore, EMiniGameId miniGameId);
    virtual ~ClientboundPreLoginPacket();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create();

private:
    friend class CPlatformNetworkManagerNintendo;
    friend class FakeClientPacketListener;
    friend class ClientPacketListener;

    PlayerUID* mPlayers;
    unsigned int mPlayerCount;
    unsigned char mGameMode;
    unsigned int mSeed;
    char mUniqueMapName[16];
    unsigned int mHostPlayer;
    unsigned char mDifficulty;
    unsigned int mGameType;
    short mProtocolVersion;
    bool mHardcore;
    EMiniGameId mMiniGameId;
    std::wstring mName;
    unsigned char mMaxPlayers;
};

#endif
