#include "game/network/ClientboundUpdateAttributesPacket.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundUpdateAttributesPacket::AttributeSnapshot::AttributeSnapshot(
    eATTRIBUTE_ID attributeId, double baseValue, boost::unordered_set<AttributeModifier*>* sourceModifiers)
    : id(attributeId),
      base(baseValue),
      modifiers()
{
    if (sourceModifiers != 0)
    {
        for (boost::unordered_set<AttributeModifier*>::iterator it = sourceModifiers->begin();
             it != sourceModifiers->end(); ++it)
        {
            modifiers.insert(new AttributeModifier(**it));
        }
    }
}

ClientboundUpdateAttributesPacket::AttributeSnapshot::~AttributeSnapshot()
{
    for (boost::unordered_set<AttributeModifier*>::iterator it = modifiers.begin(); it != modifiers.end(); ++it)
        delete *it;
}

boost::unordered_set<AttributeModifier*>* ClientboundUpdateAttributesPacket::AttributeSnapshot::getModifiers()
{
    return &modifiers;
}
eATTRIBUTE_ID ClientboundUpdateAttributesPacket::AttributeSnapshot::getId()
{
    return id;
}
double ClientboundUpdateAttributesPacket::AttributeSnapshot::getBase()
{
    return base;
}

ClientboundUpdateAttributesPacket::ClientboundUpdateAttributesPacket()
{
    entityId = 0;
}

ClientboundUpdateAttributesPacket::ClientboundUpdateAttributesPacket(
    int idValue, int typeValue, boost::unordered_set<AttributeInstance*>* attributes)
    : entityId(idValue),
      entityType(typeValue),
      values()
{
    if (attributes == 0)
        return;
    for (boost::unordered_set<AttributeInstance*>::iterator it = attributes->begin(); it != attributes->end(); ++it)
    {
        AttributeInstance* attribute = *it;
        if (attribute != 0)
        {
            boost::unordered_set<AttributeModifier*> modifiers;
            attribute->getModifiers(modifiers);
            values.insert(
                new AttributeSnapshot(attribute->getAttribute()->getId(), attribute->getBaseValue(), &modifiers));
        }
    }
}

ClientboundUpdateAttributesPacket::~ClientboundUpdateAttributesPacket()
{
    for (boost::unordered_set<AttributeSnapshot*>::iterator it = values.begin(); it != values.end(); ++it)
        delete *it;
}

void ClientboundUpdateAttributesPacket::read(DataInputStream* input)
{
    entityId = input->readUnsignedShort();
    entityType = input->readInt();
    int count = input->readByte();
    for (int i = 0; i < count; ++i)
    {
        eATTRIBUTE_ID id = static_cast<eATTRIBUTE_ID>(input->readByte());
        double baseValue = input->readDouble();
        boost::unordered_set<AttributeModifier*> modifiers;
        int modifierCount = input->readByte();
        for (int j = 0; j < modifierCount; ++j)
        {
            eMODIFIER_ID modifierId = static_cast<eMODIFIER_ID>(input->readByte());
            double amount = input->readDouble();
            int operation = input->readByte();
            modifiers.insert(new AttributeModifier(modifierId, amount, operation));
        }
        values.insert(new AttributeSnapshot(id, baseValue, &modifiers));
        for (boost::unordered_set<AttributeModifier*>::iterator it = modifiers.begin(); it != modifiers.end(); ++it)
            delete *it;
    }
}

void ClientboundUpdateAttributesPacket::write(DataOutputStream* output)
{
    output->writeUnsignedShort(static_cast<unsigned short>(entityId));
    output->writeInt(entityType);
    output->writeByte(static_cast<unsigned char>(values.size()));
    for (boost::unordered_set<AttributeSnapshot*>::iterator it = values.begin(); it != values.end(); ++it)
    {
        AttributeSnapshot* value = *it;
        boost::unordered_set<AttributeModifier*>* modifiers = value->getModifiers();
        output->writeByte(static_cast<unsigned char>(value->getId()));
        output->writeDouble(value->getBase());
        output->writeByte(static_cast<unsigned char>(modifiers->size()));
        for (boost::unordered_set<AttributeModifier*>::iterator modifier = modifiers->begin();
             modifier != modifiers->end(); ++modifier)
        {
            AttributeModifier* entry = *modifier;
            output->writeByte(static_cast<unsigned char>(entry->getId()));
            output->writeDouble(entry->getAmount());
            output->writeByte(static_cast<unsigned char>(entry->getOperation()));
        }
    }
}

void ClientboundUpdateAttributesPacket::handle(PacketListener* listener)
{
    listener->handleUpdateAttributes(shared_from_this());
}
int ClientboundUpdateAttributesPacket::getEstimatedSize()
{
    return values.size() * 24 + 8;
}
int ClientboundUpdateAttributesPacket::getEntityId()
{
    return entityId;
}
int ClientboundUpdateAttributesPacket::getEntityType()
{
    return entityType;
}
boost::unordered_set<ClientboundUpdateAttributesPacket::AttributeSnapshot*> ClientboundUpdateAttributesPacket::
    getValues()
{
    return values;
}
int ClientboundUpdateAttributesPacket::getPacketId()
{
    return 44;
}
boost::shared_ptr<Packet> ClientboundUpdateAttributesPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundUpdateAttributesPacket());
}
