#ifndef GAME_NETWORK_CLIENTBOUNDREMOVEENTITIESPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDREMOVEENTITIESPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ClientPacketListener;

class ClientboundRemoveEntitiesPacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundRemoveEntitiesPacket>
{
public:
    ClientboundRemoveEntitiesPacket();
    explicit ClientboundRemoveEntitiesPacket(int entityId);
    explicit ClientboundRemoveEntitiesPacket(arrayWithLength<int> entityIds);
    virtual ~ClientboundRemoveEntitiesPacket();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundRemoveEntitiesPacket());
    }

private:
    friend class ClientPacketListener;
    arrayWithLength<int> entityIds;
};

#endif
