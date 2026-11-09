#ifndef GAME_NETWORK_CLIENTBOUNDUPDATEATTRIBUTESPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDUPDATEATTRIBUTESPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <string>

#include "game/core/Attribute.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/network/Packet.hpp"

class AttributeInstance;
class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundUpdateAttributesPacket : public Packet,
                                          public boost::enable_shared_from_this<ClientboundUpdateAttributesPacket>
{
public:
    class AttributeSnapshot
    {
    public:
        AttributeSnapshot(eATTRIBUTE_ID id, double base, boost::unordered_set<AttributeModifier*>* modifiers);
        ~AttributeSnapshot();
        boost::unordered_set<AttributeModifier*>* getModifiers();
        eATTRIBUTE_ID getId();
        double getBase();

    private:
        eATTRIBUTE_ID id;
        double base;
        boost::unordered_set<AttributeModifier*> modifiers;
    };

    ClientboundUpdateAttributesPacket();
    ClientboundUpdateAttributesPacket(int entityId, int entityType,
                                      boost::unordered_set<AttributeInstance*>* attributes);
    virtual ~ClientboundUpdateAttributesPacket();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    int getEntityId();
    int getEntityType();
    boost::unordered_set<AttributeSnapshot*> getValues();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();

private:
    int entityId;
    int entityType;
    boost::unordered_set<AttributeSnapshot*> values;
};

#endif
