#ifndef GAME_NETWORK_CLIENTBOUNDADDMOBPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDADDMOBPACKET_HPP

#include <string>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/network/Packet.hpp"

class CompoundTag;
class DataInputStream;
class DataOutputStream;
class LivingEntity;
class PacketListener;

class ClientboundAddMobPacket : public Packet, public boost::enable_shared_from_this<ClientboundAddMobPacket>
{
public:
    ClientboundAddMobPacket();
    ClientboundAddMobPacket(boost::shared_ptr<LivingEntity> entity, int yRot, int xRot, int x, int y, int z,
                            int yHeadRot);
    ClientboundAddMobPacket(boost::shared_ptr<LivingEntity> entity, int yRot, int xRot, int x, int y, int z,
                            int yHeadRot, CompoundTag* additionalContructionDataTag);
    virtual ~ClientboundAddMobPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    std::vector<DataItem*>* getUnpackedData();
    int getId();
    std::wstring getUUID();
    int getType();
    int getX();
    int getY();
    int getZ();
    int getXd();
    int getYd();
    int getZd();
    unsigned char getyRot();
    unsigned char getxRot();
    unsigned char getyHeadRot();
    std::vector<int> getSubIds();
    CompoundTag* getAdditionalContructionDataTag();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundAddMobPacket()); }

private:
    void init(boost::shared_ptr<LivingEntity> entity, int yRot, int xRot, int x, int y, int z, int yHeadRot,
              CompoundTag* additionalContructionDataTag);

    int id;
    std::wstring uuid;
    int type;
    int x;
    int y;
    int z;
    int xd;
    int yd;
    int zd;
    unsigned char yRot;
    unsigned char xRot;
    unsigned char yHeadRot;
    std::vector<int> subIds;
    CompoundTag* additionalContructionDataTag;
    boost::shared_ptr<SynchedEntityData> entityData;
    std::vector<DataItem*>* unpackedData;
};

#endif
