#ifndef GAME_NETWORK_CLIENTBOUNDRESPAWNPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDRESPAWNPACKET_HPP

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Difficulty;
class GameType;
class LevelType;
class PacketListener;
class ClientPacketListener;

class ClientboundRespawnPacket : public Packet, public boost::enable_shared_from_this<ClientboundRespawnPacket>
{
    friend class ClientPacketListener;

public:
    ClientboundRespawnPacket();
    ClientboundRespawnPacket(char dimension, long long mapSeed, const GameType* playerGameType,
                             const Difficulty* difficulty, LevelType* levelType, bool newSeaLevel, int newEntityId,
                             int xzSize, int hellScale, int cloudHeight, bool modernEnd, unsigned char biomeScale,
                             int biomeCentreXChunk, int biomeCentreZChunk, bool keepOldEntityData, int seaLevel);
    ClientboundRespawnPacket(double x, double y, double z, float yRot, float xRot, char dimension, long long mapSeed,
                             const GameType* playerGameType, const Difficulty* difficulty, LevelType* levelType,
                             bool newSeaLevel, int newEntityId, int xzSize, int hellScale, int cloudHeight,
                             bool modernEnd, unsigned char biomeScale, int biomeCentreXChunk, int biomeCentreZChunk,
                             bool keepOldEntityData, int seaLevel);
    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    int getDimension();
    const Difficulty* getDifficulty();
    const GameType* getPlayerGameType();
    LevelType* getLevelType();
    bool shouldKeepOldEntityData() const;
    bool hasPositionAndRotation() const;
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundRespawnPacket()); }

private:
    long long m_mapSeed;
    bool m_newSeaLevel;
    int m_seaLevel;
    int m_newEntityId;
    int m_xzSize;
    int m_hellScale;
    int m_cloudHeight;
    bool m_modernEnd;
    unsigned char m_biomeScale;
    int m_biomeCentreXChunk;
    int m_biomeCentreZChunk;
    double m_x;
    double m_y;
    double m_z;
    float m_yRot;
    float m_xRot;
    unsigned char m_keepOldEntityData;
    char m_dimension;
    const Difficulty* m_difficulty;
    const GameType* m_playerGameType;
    LevelType* m_levelType;
    bool m_hasPositionAndRotation;
};

#endif
