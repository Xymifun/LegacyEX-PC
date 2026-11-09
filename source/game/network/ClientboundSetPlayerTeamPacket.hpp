#ifndef GAME_NETWORK_CLIENTBOUNDSETPLAYERTEAMPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETPLAYERTEAMPACKET_HPP

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/generation/PlayerUID.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class PlayerTeam;

class ClientboundSetPlayerTeamPacket : public Packet,
                                       public boost::enable_shared_from_this<ClientboundSetPlayerTeamPacket>
{
public:
    ClientboundSetPlayerTeamPacket();
    ClientboundSetPlayerTeamPacket(PlayerTeam* team, int method);
    ClientboundSetPlayerTeamPacket(PlayerTeam* team, std::vector<PlayerUID>* players, int method);

    std::wstring getName();
    std::vector<PlayerUID>* getPlayers();
    int getMethod();
    int getOptions();
    int getNameTagVisibility();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetPlayerTeamPacket());
    }

private:
    std::wstring mName;
    std::vector<PlayerUID> mPlayers;
    int mMethod;
    int mOptions;
    int mNameTagVisibility;
};

#endif
