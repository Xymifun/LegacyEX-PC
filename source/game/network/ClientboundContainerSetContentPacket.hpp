#ifndef GAME_NETWORK_CLIENTBOUNDCONTAINERSETCONTENTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCONTAINERSETCONTENTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/item/ItemInstance.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundContainerSetContentPacket : public Packet,
                                             public boost::enable_shared_from_this<ClientboundContainerSetContentPacket>
{
public:
    ClientboundContainerSetContentPacket();
    ClientboundContainerSetContentPacket(int containerId, std::vector<not_null_ptr<ItemInstance> >* items);
    virtual ~ClientboundContainerSetContentPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getContainerId();
    NonNullList<not_null_ptr<ItemInstance> > getItems();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundContainerSetContentPacket());
    }

private:
    int containerId;
    NonNullList<not_null_ptr<ItemInstance> > items;
};

#endif
