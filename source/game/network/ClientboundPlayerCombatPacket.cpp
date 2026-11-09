#include "game/network/ClientboundPlayerCombatPacket.hpp"

#include "game/core/CombatTracker.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundPlayerCombatPacket::ClientboundPlayerCombatPacket()
{
}

ClientboundPlayerCombatPacket::ClientboundPlayerCombatPacket(CombatTracker* combatTracker, Event event)
{
    _init(combatTracker, event, true);
}

ClientboundPlayerCombatPacket::ClientboundPlayerCombatPacket(CombatTracker* combatTracker, Event event,
                                                             bool includeKiller)
{
    _init(combatTracker, event, includeKiller);
}

ClientboundPlayerCombatPacket::~ClientboundPlayerCombatPacket()
{
}

void ClientboundPlayerCombatPacket::_init(CombatTracker* combatTracker, Event event, bool includeKiller)
{
    mEvent = event;
    boost::shared_ptr<LivingEntity> killer = combatTracker->getKiller();
    switch (event)
    {
    case END_COMBAT:
        mDuration = combatTracker->getCombatDuration();
        mKillerId = !killer ? -1 : killer->getId();
        break;
    case ENTITY_DIED:
        mEntityId = combatTracker->getMob()->getId();
        mKillerId = !killer ? -1 : killer->getId();
        mMessage = L"";
        break;
    }
}

void ClientboundPlayerCombatPacket::handle(PacketListener* listener)
{
    listener->handlePlayerCombat(shared_from_this());
}

void ClientboundPlayerCombatPacket::read(DataInputStream* input)
{
    mEvent = static_cast<Event>(input->readInt());
    if (mEvent == END_COMBAT)
    {
        mDuration = input->readVarInt();
        mKillerId = input->readInt();
    }
    else if (mEvent == ENTITY_DIED)
    {
        mEntityId = input->readVarInt();
        mKillerId = input->readInt();
        mMessage = Packet::readUtf(input, 32767);
    }
}

void ClientboundPlayerCombatPacket::write(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(mEvent));
    if (mEvent == END_COMBAT)
    {
        output->writeVarInt(mDuration);
        output->writeInt(mKillerId);
    }
    else if (mEvent == ENTITY_DIED)
    {
        output->writeVarInt(mEntityId);
        output->writeInt(mKillerId);
        Packet::writeUtf(mMessage, output);
    }
}

int ClientboundPlayerCombatPacket::getPacketId()
{
    return 213;
}
