#ifndef GAME_NETWORK_SCOREBOARDPACKET_HPP
#define GAME_NETWORK_SCOREBOARDPACKET_HPP

#include <stdint.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/StatsUID.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ScoreboardPacket : public Packet, public boost::enable_shared_from_this<ScoreboardPacket>
{
public:
    enum EMessage
    {
        ADD_PLAYER_STATS,
        UPDATE_PLAYER_STATS,
        REMOVE_PLAYER_STATS,
        CLEAR_PLAYER_STATS
    };

    class Data
    {
    public:
        virtual ~Data();
        virtual void read(DataInputStream* input);
        virtual void write(DataOutputStream* output) const;
    };

    class AddPlayerStatsData : public Data
    {
    public:
        AddPlayerStatsData();
        AddPlayerStatsData(Scoreboard::PlayerStats* stats, int round, int life);
        virtual ~AddPlayerStatsData();
        virtual void read(DataInputStream* input);
        virtual void write(DataOutputStream* output) const;
        Scoreboard::PlayerStats* mStats;
        int mRound;
        int mLife;
    };

    class UpdatePlayerStatsData : public Data
    {
    public:
        UpdatePlayerStatsData();
        UpdatePlayerStatsData(Scoreboard::PlayerStats* stats, Scoreboard::PlayerStats* other, int round, int life);
        virtual ~UpdatePlayerStatsData();
        virtual void read(DataInputStream* input);
        virtual void write(DataOutputStream* output) const;
        Scoreboard::PlayerStats* mStats;
        int mRound;
        int mLife;
        unsigned long long mChanged;
    };

    class RemovePlayerStatsData : public Data
    {
    public:
        RemovePlayerStatsData();
        RemovePlayerStatsData(StatsUID uid);
        virtual ~RemovePlayerStatsData();
        virtual void read(DataInputStream* input);
        virtual void write(DataOutputStream* output) const;
        StatsUID mUID;
    };

    ScoreboardPacket();
    ScoreboardPacket(EMessage message, Data* data);
    virtual ~ScoreboardPacket();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ScoreboardPacket()); }
    EMessage GetMessage();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    Data* getData();
    virtual int getPacketId();

private:
    EMessage mMessage;
    Data* mData;
};

#endif
