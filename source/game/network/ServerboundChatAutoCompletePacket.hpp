#ifndef GAME_NETWORK_SERVERBOUNDCHATAUTOCOMPLETEPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCHATAUTOCOMPLETEPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundChatAutoCompletePacket : public Packet,
                                          public boost::enable_shared_from_this<ServerboundChatAutoCompletePacket>
{
public:
    ServerboundChatAutoCompletePacket();
    ServerboundChatAutoCompletePacket(const std::wstring& message, const BlockPos& lookAtPos);
    ServerboundChatAutoCompletePacket(const std::wstring& message, const BlockPos& lookAtPos, bool assumeCommand);
    virtual ~ServerboundChatAutoCompletePacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    std::wstring getMessage();
    BlockPos* getLookAtPos();
    bool assumeCommand();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundChatAutoCompletePacket());
    }

private:
    std::wstring message;
    bool command;
    BlockPos* lookAtPos;
};

#endif
