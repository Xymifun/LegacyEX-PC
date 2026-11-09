#ifndef GAME_NETWORK_SERVERBOUNDCUSTOMPAYLOADPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCUSTOMPAYLOADPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundCustomPayloadPacket : public Packet,
                                       public boost::enable_shared_from_this<ServerboundCustomPayloadPacket>
{
public:
    ServerboundCustomPayloadPacket();
    ServerboundCustomPayloadPacket(const std::wstring& identifier, arrayWithLength<unsigned char> data);
    virtual ~ServerboundCustomPayloadPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    std::wstring getIdentifier();
    arrayWithLength<unsigned char> getData();
#ifdef MINECRAFT_PC
    unsigned int PcOversizedBytes() const { return mPcOversizedBytes; }
#endif

    static const std::wstring CUSTOM_BOOK_PACKET;
    static const std::wstring CUSTOM_BOOK_SIGN_PACKET;
    static const std::wstring TRADER_SELECTION_PACKET;
    static const std::wstring SET_ADVENTURE_COMMAND_PACKET;
    static const std::wstring SET_AUTO_COMMAND_PACKET;
    static const std::wstring SET_STRUCTURE_PACKET;
    static const std::wstring SET_BEACON_PACKET;
    static const std::wstring SET_ITEM_NAME_PACKET;
    static const std::wstring SET_RENAME_ITEM_PACKET;
    static const std::wstring PICK_ITEM_PACKET;
    static const std::wstring CLIENT_BRAND_PACKET;

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket());
    }

private:
    std::wstring identifier;
    arrayWithLength<unsigned char> data;
#ifdef MINECRAFT_PC
    unsigned int mPcOversizedBytes;
#endif
};

#endif
