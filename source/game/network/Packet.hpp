#ifndef GAME_NETWORK_PACKET_HPP
#define GAME_NETWORK_PACKET_HPP

#include <deque>
#include <stdint.h>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

class DataInputStream;
class DataOutputStream;
class PacketListener;

class Packet
{
public:
    Packet();
    virtual ~Packet() {}

    virtual int getPacketId() = 0;
    virtual void read(DataInputStream* input) = 0;
    virtual void write(DataOutputStream* output) = 0;
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual bool isAync();
    virtual bool tryReplaceDuplicatePacket(
        std::deque<boost::shared_ptr<Packet>, std::allocator<boost::shared_ptr<Packet> > >* queuedPackets);

    static void staticCtor();

    static boost::shared_ptr<Packet> readPacket(DataInputStream* input, bool isServer, unsigned int& packetsRead,
                                                bool& protocolError);
#ifdef MINECRAFT_PC
    static boost::shared_ptr<Packet> readPacket(DataInputStream* input, bool isServer, unsigned int& packetsRead,
                                                bool& protocolError, bool inGame);
#endif
    static void writePacket(boost::shared_ptr<Packet> packet, DataOutputStream* output, bool isServer,
                            unsigned int& packetsWritten);
    static bool canSendToAnyClient(boost::shared_ptr<Packet> packet);

    static void writeUtf(const std::wstring& text, DataOutputStream* output);
    static std::wstring readUtf(DataInputStream* input, int maxLength);

public:
    long long creationTime;
    bool shouldDelay;

private:
    static void map(int packetId, bool clientbound, bool serverbound, bool sendToAnyClient, bool canBeDelayed,
                    const std::type_info& type, boost::shared_ptr<Packet> (*create)(), std::wstring name);
    static boost::shared_ptr<Packet> getPacket(int packetId);
};

uint32_t packInt(int32_t value, uint32_t offset, uint32_t width);
int32_t unpackSigned(uint32_t value, uint32_t offset, uint32_t width);
uint32_t unpackUnsigned(uint32_t value, uint32_t offset, uint32_t width);

#endif
