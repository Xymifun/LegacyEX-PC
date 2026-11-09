#ifndef GAME_NETWORK_CLIENTBOUNDMGPLAYERSETTINGSUPDATEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDMGPLAYERSETTINGSUPDATEPACKET_HPP
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "game/network/Packet.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
class DataInputStream;
class DataOutputStream;
class PacketListener;
class ClientboundMGPlayerSettingsUpdatePacket
    : public Packet,
      public boost::enable_shared_from_this<ClientboundMGPlayerSettingsUpdatePacket>
{
public:
    ClientboundMGPlayerSettingsUpdatePacket();
    ClientboundMGPlayerSettingsUpdatePacket(const MiniGamePlayerSettings& settings);
    virtual ~ClientboundMGPlayerSettingsUpdatePacket();

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    const MiniGamePlayerSettings& getSettings() const;
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();

private:
    MiniGamePlayerSettings mSettings;
};
#endif
