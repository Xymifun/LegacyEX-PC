#ifndef GAME_NETWORK_CLIENTBOUNDADDPLAYERPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDADDPLAYERPACKET_HPP

#include <string>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/network/Packet.hpp"
#include "game/world/generation/PlayerUID.hpp"

class DataInputStream;
class DataOutputStream;
class GameType;
class PacketListener;
class Player;

class ClientboundAddPlayerPacket : public Packet, public boost::enable_shared_from_this<ClientboundAddPlayerPacket>
{
    friend class ClientPacketListener;

public:
    ClientboundAddPlayerPacket();
    ClientboundAddPlayerPacket(boost::shared_ptr<Player> player, PlayerUID playerId, PlayerUID profileId, int x, int y,
                               int z, int yRot, int xRot, int yHeadRot);
    virtual ~ClientboundAddPlayerPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    unsigned int getEntityId();
    PlayerUID getPlayerId();
    int getX();
    int getY();
    int getZ();
    unsigned char getyRot();
    unsigned char getxRot();
    std::wstring getName();
    std::vector<DataItem*>* getUnpackedData();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundAddPlayerPacket()); }

private:
    unsigned int mEntityId;
    PlayerUID mPlayerId;
    std::wstring mName;
    int mX;
    int mY;
    int mZ;
    unsigned char myRot;
    unsigned char mxRot;
    PlayerUID mProfileId;
    unsigned char mPlayerIndex;
    int mSkinId;
    int mCapeId;
    int mPlayerPrivilege;
    unsigned char myHeadRot;
    const GameType* mGameType;
    boost::shared_ptr<SynchedEntityData> mEntityData;
    std::vector<DataItem*>* mUnpackedData;
};

#endif
