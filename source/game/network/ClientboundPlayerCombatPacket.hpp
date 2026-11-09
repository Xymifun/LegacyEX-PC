#ifndef GAME_NETWORK_CLIENTBOUNDPLAYERCOMBATPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDPLAYERCOMBATPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class CombatTracker;
class DataInputStream;
class DataOutputStream;
class LivingEntity;
class PacketListener;

class ClientboundPlayerCombatPacket : public Packet,
                                      public boost::enable_shared_from_this<ClientboundPlayerCombatPacket>
{
public:
    enum Event
    {
        ENTER_COMBAT,
        END_COMBAT,
        ENTITY_DIED
    };

    ClientboundPlayerCombatPacket();
    ClientboundPlayerCombatPacket(CombatTracker* combatTracker, Event event);
    ClientboundPlayerCombatPacket(CombatTracker* combatTracker, Event event, bool includeKiller);
    virtual ~ClientboundPlayerCombatPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundPlayerCombatPacket()); }

private:
    void _init(CombatTracker* combatTracker, Event event, bool includeKiller);

    Event mEvent;
    int mEntityId;
    int mKillerId;
    int mDuration;
    std::wstring mMessage;
};

#endif
