#include "game/network/ScoreboardPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/core/StatsUID.hpp"

ScoreboardPacket::ScoreboardPacket() : mData(0)
{
}

ScoreboardPacket::ScoreboardPacket(EMessage message, Data* data) : mMessage(message), mData(data)
{
}

ScoreboardPacket::~ScoreboardPacket()
{
    delete mData;
}

ScoreboardPacket::EMessage ScoreboardPacket::GetMessage()
{
    return mMessage;
}

void ScoreboardPacket::read(DataInputStream* input)
{
    mMessage = static_cast<EMessage>(input->readByte());
    switch (GetMessage())
    {
    case ADD_PLAYER_STATS:
        mData = new AddPlayerStatsData();
        break;
    case UPDATE_PLAYER_STATS:
        mData = new UpdatePlayerStatsData();
        break;
    case REMOVE_PLAYER_STATS:
        mData = new RemovePlayerStatsData();
        break;
    case CLEAR_PLAYER_STATS:
        mData = new Data();
        break;
    default:
        return;
    }
    mData->read(input);
}

void ScoreboardPacket::write(DataOutputStream* output)
{
    output->writeByte(static_cast<unsigned char>(mMessage));
    mData->write(output);
}

void ScoreboardPacket::handle(PacketListener* listener)
{
    listener->handleScoreboardPacket(shared_from_this());
}

int ScoreboardPacket::getEstimatedSize()
{
    return Packet::getEstimatedSize() + 1;
}

ScoreboardPacket::Data* ScoreboardPacket::getData()
{
    return mData;
}

int ScoreboardPacket::getPacketId()
{
    return 206;
}

ScoreboardPacket::Data::~Data()
{
}
void ScoreboardPacket::Data::read(DataInputStream*)
{
}
void ScoreboardPacket::Data::write(DataOutputStream*) const
{
}

ScoreboardPacket::AddPlayerStatsData::AddPlayerStatsData() : mStats(0), mRound(-1), mLife(-1)
{
}

ScoreboardPacket::AddPlayerStatsData::AddPlayerStatsData(Scoreboard::PlayerStats* stats, int round, int life)
    : mStats(stats),
      mRound(round),
      mLife(life)
{
}

ScoreboardPacket::AddPlayerStatsData::~AddPlayerStatsData()
{
    delete mStats;
}

void ScoreboardPacket::AddPlayerStatsData::read(DataInputStream* input)
{
    mStats = new Scoreboard::PlayerStats();
    mRound = input->readByte();
    mLife = input->readByte();
    int record;
    for (record = 0; record < Scoreboard::PlayerStats::eRecord_Count; ++record)
        mStats->readRecord(input, record, mRound, mLife);
}

void ScoreboardPacket::AddPlayerStatsData::write(DataOutputStream* output) const
{
    output->writeByte(static_cast<unsigned char>(mRound));
    output->writeByte(static_cast<unsigned char>(mLife));
    int record;
    for (record = 0; record < Scoreboard::PlayerStats::eRecord_Count; ++record)
        mStats->writeRecord(output, record, mRound, mLife);
}

ScoreboardPacket::UpdatePlayerStatsData::UpdatePlayerStatsData()
{
    mStats = 0;
    mRound = -1;
    mChanged = 0;
    mLife = -1;
}

ScoreboardPacket::UpdatePlayerStatsData::UpdatePlayerStatsData(Scoreboard::PlayerStats* stats,
                                                               Scoreboard::PlayerStats* other, int round, int life)
    : mStats(stats),
      mRound(round),
      mLife(life),
      mChanged(stats->getChangedBitfield(other, round, life) | 1ULL)
{
}

ScoreboardPacket::UpdatePlayerStatsData::~UpdatePlayerStatsData()
{
    delete mStats;
}

void ScoreboardPacket::UpdatePlayerStatsData::read(DataInputStream* input)
{
    mStats = new Scoreboard::PlayerStats();
    mRound = input->readByte();
    mLife = input->readByte();
    mChanged = static_cast<unsigned long long>(input->readLong());
    int record;
    for (record = 0; record < Scoreboard::PlayerStats::eRecord_Count; ++record)
        if (((mChanged >> record) & 1ULL) != 0)
            mStats->readRecord(input, record, mRound, mLife);
}

void ScoreboardPacket::UpdatePlayerStatsData::write(DataOutputStream* output) const
{
    output->writeByte(static_cast<unsigned char>(mRound));
    output->writeByte(static_cast<unsigned char>(mLife));
    output->writeLong(static_cast<long long>(mChanged));
    int record;
    for (record = 0; record < Scoreboard::PlayerStats::eRecord_Count; ++record)
        if (((mChanged >> record) & 1ULL) != 0)
            mStats->writeRecord(output, record, mRound, mLife);
}

ScoreboardPacket::RemovePlayerStatsData::RemovePlayerStatsData()
{
    mUID = StatsUID::EMPTY_SUID();
}

ScoreboardPacket::RemovePlayerStatsData::RemovePlayerStatsData(StatsUID uid)
{
    mUID = uid;
}

ScoreboardPacket::RemovePlayerStatsData::~RemovePlayerStatsData()
{
}

void ScoreboardPacket::RemovePlayerStatsData::read(DataInputStream* input)
{
    mUID = StatsUID(input->readPlayerUID());
}

void ScoreboardPacket::RemovePlayerStatsData::write(DataOutputStream* output) const
{
    output->writePlayerUID(mUID.GetID());
}
