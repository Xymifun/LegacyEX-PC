#ifndef GAME_NETWORK_CLIENTBOUNDSETENTITYDATAPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETENTITYDATAPACKET_HPP

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataItem;
class DataOutputStream;
class PacketListener;
class SynchedEntityData;

class ClientboundSetEntityDataPacket : public Packet,
                                       public boost::enable_shared_from_this<ClientboundSetEntityDataPacket>
{
public:
    ClientboundSetEntityDataPacket();
    ClientboundSetEntityDataPacket(int id, int entityType, boost::shared_ptr<SynchedEntityData> entityData,
                                   bool sendAll);
    virtual ~ClientboundSetEntityDataPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getId();
    int getEntityType();
    std::vector<DataItem*>* getUnpackedData();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetEntityDataPacket());
    }

private:
    boost::shared_ptr<SynchedEntityData> entityData;
    std::vector<DataItem*>* unpackedData;
    int id;
    int entityType;
};

#endif
