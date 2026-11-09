#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/LevelType.hpp"

ClientboundRespawnPacket::ClientboundRespawnPacket()
{
    m_dimension = 0;
    m_mapSeed = 0;
    m_y = 0.0;
    m_biomeCentreXChunk = 0;
    m_z = 0.0;
    m_modernEnd = true;
    m_x = 0.0;
    m_newSeaLevel = false;
    m_hellScale = 3;
    m_xzSize = 54;
    m_yRot = 0.0f;
    m_cloudHeight = 128;
    m_biomeScale = 0;
    m_difficulty = NULL;
    m_playerGameType = NULL;
    m_hasPositionAndRotation = false;
    m_keepOldEntityData = true;
    m_seaLevel = -1;
    m_xRot = 0.0f;
    m_newEntityId = 0;
    m_biomeCentreZChunk = 0;
    m_levelType = NULL;
}

ClientboundRespawnPacket::ClientboundRespawnPacket(char dimension, long long mapSeed, const GameType* playerGameType,
                                                   const Difficulty* difficulty, LevelType* levelType, bool newSeaLevel,
                                                   int newEntityId, int xzSize, int hellScale, int cloudHeight,
                                                   bool modernEnd, unsigned char biomeScale, int biomeCentreXChunk,
                                                   int biomeCentreZChunk, bool keepOldEntityData, int seaLevel)
{
    m_dimension = dimension;
    m_mapSeed = mapSeed;
    m_x = 0.0;
    m_xRot = 0.0f;
    m_yRot = 0.0f;
    m_keepOldEntityData = keepOldEntityData;
    m_difficulty = difficulty;
    m_newSeaLevel = newSeaLevel;
    m_biomeCentreXChunk = biomeCentreXChunk;
    m_hasPositionAndRotation = false;
    m_biomeCentreZChunk = biomeCentreZChunk;
    m_cloudHeight = cloudHeight;
    m_modernEnd = modernEnd;
    m_newEntityId = newEntityId;
    m_xzSize = xzSize;
    m_z = 0.0;
    m_playerGameType = playerGameType;
    m_hellScale = hellScale;
    m_y = 0.0;
    m_seaLevel = seaLevel;
    m_biomeScale = biomeScale;
    m_levelType = levelType;
}

ClientboundRespawnPacket::ClientboundRespawnPacket(double x, double y, double z, float yRot, float xRot, char dimension,
                                                   long long mapSeed, const GameType* playerGameType,
                                                   const Difficulty* difficulty, LevelType* levelType, bool newSeaLevel,
                                                   int newEntityId, int xzSize, int hellScale, int cloudHeight,
                                                   bool modernEnd, unsigned char biomeScale, int biomeCentreXChunk,
                                                   int biomeCentreZChunk, bool keepOldEntityData, int seaLevel)
{
    m_keepOldEntityData = keepOldEntityData;
    m_mapSeed = mapSeed;
    m_z = z;
    m_yRot = yRot;
    m_hasPositionAndRotation = true;
    m_cloudHeight = cloudHeight;
    m_biomeScale = biomeScale;
    m_x = x;
    m_newEntityId = newEntityId;
    m_xzSize = xzSize;
    m_newSeaLevel = newSeaLevel;
    m_difficulty = difficulty;
    m_seaLevel = seaLevel;
    m_biomeCentreXChunk = biomeCentreXChunk;
    m_modernEnd = modernEnd;
    m_dimension = dimension;
    m_xRot = xRot;
    m_y = y;
    m_playerGameType = playerGameType;
    m_biomeCentreZChunk = biomeCentreZChunk;
    m_levelType = levelType;
    m_hellScale = hellScale;
}

void ClientboundRespawnPacket::handle(PacketListener* listener)
{
    listener->handleRespawn(shared_from_this());
}

void ClientboundRespawnPacket::read(DataInputStream* input)
{
    m_dimension = input->readByte();
    m_playerGameType = GameType::byId(input->readUnsignedByte());
    std::wstring levelTypeName = Packet::readUtf(input, 16);
    m_levelType = LevelType::getLevelType(levelTypeName);
    if (m_levelType == NULL)
        m_levelType = LevelType::DEFAULT;
    m_mapSeed = input->readLong();
    m_difficulty = Difficulty::byId(input->readUnsignedByte());
    m_newSeaLevel = input->readBoolean();
    m_newEntityId = input->readShort();
    m_cloudHeight = input->readInt();
    m_modernEnd = input->readBoolean();
    m_biomeScale = input->readByte();
    m_biomeCentreXChunk = input->readInt();
    m_biomeCentreZChunk = input->readInt();
    m_keepOldEntityData = input->read();
    m_hasPositionAndRotation = input->readBoolean();
    if (m_hasPositionAndRotation)
    {
        m_x = input->readDouble();
        m_y = input->readDouble();
        m_z = input->readDouble();
        m_xRot = input->readFloat();
        m_yRot = input->readFloat();
    }
    m_seaLevel = input->readInt();
#ifdef MINECRAFT_PC
    m_xzSize = input->readShort();
    m_hellScale = input->readByte();
#endif
}

void ClientboundRespawnPacket::write(DataOutputStream* output)
{
    output->writeByte(m_dimension);
    output->writeByte(m_playerGameType->getId());
    if (m_levelType == NULL)
        Packet::writeUtf(L"", output);
    else
        Packet::writeUtf(m_levelType->getGeneratorName(), output);
    output->writeLong(m_mapSeed);
    output->writeByte(m_difficulty->getId());
    output->writeBoolean(m_newSeaLevel);
    output->writeShort(static_cast<short>(m_newEntityId));
    output->writeInt(m_cloudHeight);
    output->writeBoolean(m_modernEnd);
    output->writeByte(m_biomeScale);
    output->writeInt(m_biomeCentreXChunk);
    output->writeInt(m_biomeCentreZChunk);
    output->write(m_keepOldEntityData);
    output->writeBoolean(m_hasPositionAndRotation);
    if (m_hasPositionAndRotation)
    {
        output->writeDouble(m_x);
        output->writeDouble(m_y);
        output->writeDouble(m_z);
        output->writeFloat(m_xRot);
        output->writeFloat(m_yRot);
    }
    output->writeInt(m_seaLevel);
#ifdef MINECRAFT_PC
    output->writeShort(static_cast<short>(m_xzSize));
    output->writeByte(m_hellScale);
#endif
}

int ClientboundRespawnPacket::getEstimatedSize()
{
    int levelTypeLength = 0;
    if (m_levelType != NULL)
        levelTypeLength = static_cast<int>(m_levelType->getGeneratorName().length());
    return levelTypeLength + 18;
}

int ClientboundRespawnPacket::getDimension()
{
    return m_dimension;
}
const Difficulty* ClientboundRespawnPacket::getDifficulty()
{
    return m_difficulty;
}
const GameType* ClientboundRespawnPacket::getPlayerGameType()
{
    return m_playerGameType;
}
LevelType* ClientboundRespawnPacket::getLevelType()
{
    return m_levelType;
}
bool ClientboundRespawnPacket::shouldKeepOldEntityData() const
{
    return m_keepOldEntityData == 1;
}
bool ClientboundRespawnPacket::hasPositionAndRotation() const
{
    return m_hasPositionAndRotation;
}
int ClientboundRespawnPacket::getPacketId()
{
    return 9;
}
