#include "game/network/GameModePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/TeamInvite.hpp"
#include "game/network/PacketListener.hpp"

void GameModePacket::EmptyData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
}

void GameModePacket::EmptyData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
}

GameModePacket::IntData::IntData() : value(0)
{
}

GameModePacket::IntData::IntData(int value) : value(value)
{
}

void GameModePacket::IntData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    value = packet->ints[0];
}

void GameModePacket::IntData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(value);
}

GameModePacket::FollowData::FollowData() : follow(false), teleport(false)
{
}

GameModePacket::FollowData::FollowData(bool follow, bool teleport) : follow(follow), teleport(teleport)
{
}

void GameModePacket::FollowData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    follow = packet->bools[0];
    teleport = packet->bools[1];
}

void GameModePacket::FollowData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->bools.push_back(follow);
    packet->bools.push_back(teleport);
}

GameModePacket::TransitionData::TransitionData() : transition(0), level(0), x(0), y(0)
{
}

GameModePacket::TransitionData::TransitionData(int transition, int level, int x, int y)
    : transition(transition),
      level(level),
      x(x),
      y(y)
{
}

void GameModePacket::TransitionData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    transition = packet->ints[0];
    level = packet->ints[1];
    x = packet->ints[2];
    y = packet->ints[3];
}

void GameModePacket::TransitionData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(transition);
    packet->ints.push_back(level);
    packet->ints.push_back(x);
    packet->ints.push_back(y);
}

GameModePacket::TransitionToRoundData::TransitionToRoundData() : TransitionData(), round(0), seed(-1), mapSize(-1)
{
}

GameModePacket::TransitionToRoundData::TransitionToRoundData(int round, int seed, int mapSize, int transition,
                                                             int level, int x, int y)
    : TransitionData(transition, level, x, y),
      round(round),
      seed(seed),
      mapSize(mapSize)
{
}

void GameModePacket::TransitionToRoundData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    TransitionData::ReadPacket(packet);
    round = packet->ints[4];
    seed = packet->ints[5];
    mapSize = packet->ints[6];
}

void GameModePacket::TransitionToRoundData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    TransitionData::WritePacket(packet);
    packet->ints.push_back(round);
    packet->ints.push_back(seed);
    packet->ints.push_back(mapSize);
}

GameModePacket::ChangeCustomGameMode::ChangeCustomGameMode() : gameMode(0), customGameMode(0)
{
}

GameModePacket::ChangeCustomGameMode::ChangeCustomGameMode(int gameMode, int customGameMode)
    : gameMode(gameMode),
      customGameMode(customGameMode)
{
}

void GameModePacket::ChangeCustomGameMode::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    gameMode = packet->ints[0];
    customGameMode = packet->ints[1];
}

void GameModePacket::ChangeCustomGameMode::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(gameMode);
    packet->ints.push_back(customGameMode);
}

GameModePacket::UpdateStartTimeData::UpdateStartTimeData() : startTime(0)
{
}

GameModePacket::UpdateStartTimeData::UpdateStartTimeData(long long startTime) : startTime(startTime)
{
}

void GameModePacket::UpdateStartTimeData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    startTime = packet->ints[0];
}

void GameModePacket::UpdateStartTimeData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(static_cast<int>(startTime));
}

GameModePacket::LockPlayerLocation::LockPlayerLocation() : locked(false)
{
}

GameModePacket::LockPlayerLocation::LockPlayerLocation(bool locked) : locked(locked)
{
}

void GameModePacket::LockPlayerLocation::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    locked = packet->bools[0];
}

void GameModePacket::LockPlayerLocation::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->bools.push_back(locked);
}

GameModePacket::ChestRefill::ChestRefill()
{
    positions = std::vector<BlockPos>();
}

GameModePacket::ChestRefill::ChestRefill(std::vector<BlockPos> positions)
{
    this->positions = positions;
}

void GameModePacket::ChestRefill::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    int coordinateCount = packet->ints[0];

    for (int index = 1; index < coordinateCount + 1; index += 3)
    {
        positions.push_back(BlockPos(packet->ints[index], packet->ints[index + 1], packet->ints[index + 2]));
    }
}

void GameModePacket::ChestRefill::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(static_cast<int>(positions.size() * 3));

    for (unsigned int index = 0; index < positions.size(); ++index)
    {
        packet->ints.push_back(positions[index].x);
        packet->ints.push_back(positions[index].y);
        packet->ints.push_back(positions[index].z);
    }
}

GameModePacket::PlayerData::PlayerData() : playerName()
{
}

GameModePacket::PlayerData::PlayerData(std::wstring playerName)
{
    this->playerName = playerName;
}

void GameModePacket::PlayerData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    playerName = packet->strings[0];
}

void GameModePacket::PlayerData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->strings.push_back(playerName);
}

void GameModePacket::SetCameraData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    camera = packet->ints[0];
    target = packet->ints[1];
}

void GameModePacket::SetCameraData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(camera);
    packet->ints.push_back(target);
}

GameModePacket::SyncTeamInviteData::SyncTeamInviteData() : invite(0)
{
}

void GameModePacket::SyncTeamInviteData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    invite = new TeamInvite();
    invite->m_uTeamID = packet->ints[0];
    invite->m_sFromPlayerName = packet->strings[0];
    invite->m_sToPlayerName = packet->strings[1];
    invite->m_sTeamName = packet->strings[2];
}

void GameModePacket::SyncTeamInviteData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(invite->m_uTeamID);
    packet->strings.push_back(invite->m_sFromPlayerName);
    packet->strings.push_back(invite->m_sToPlayerName);
    packet->strings.push_back(invite->m_sTeamName);
}

GameModePacket::TeamIDData::TeamIDData() : teamId(-1)
{
}

GameModePacket::TeamIDData::TeamIDData(int teamId) : teamId(teamId)
{
}

void GameModePacket::TeamIDData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    teamId = packet->ints[0];
}

void GameModePacket::TeamIDData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(teamId);
}

GameModePacket::AcceptTeamInviteData::~AcceptTeamInviteData()
{
}

GameModePacket::RemoveTeamInviteData::~RemoveTeamInviteData()
{
}

GameModePacket::AddAreaBoundsData::AddAreaBoundsData()
{
    minX = minY = minZ = maxX = maxY = maxZ = 0;
}

GameModePacket::AddAreaBoundsData::AddAreaBoundsData(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
    : minX(minX),
      minY(minY),
      minZ(minZ),
      maxX(maxX),
      maxY(maxY),
      maxZ(maxZ)
{
}

void GameModePacket::AddAreaBoundsData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    minX = packet->ints[0];
    minY = packet->ints[1];
    minZ = packet->ints[2];
    maxX = packet->ints[3];
    maxY = packet->ints[4];
    maxZ = packet->ints[5];
}

void GameModePacket::AddAreaBoundsData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(minX);
    packet->ints.push_back(minY);
    packet->ints.push_back(minZ);
    packet->ints.push_back(maxX);
    packet->ints.push_back(maxY);
    packet->ints.push_back(maxZ);
}

GameModePacket::StateChangeData::StateChangeData() : state(eGameModeState_None)
{
}

GameModePacket::StateChangeData::StateChangeData(EGameModeState state) : state(state)
{
}

void GameModePacket::StateChangeData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    state = static_cast<EGameModeState>(packet->ints[0]);
}

void GameModePacket::StateChangeData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(static_cast<int>(state));
}

GameModePacket::TutorialGameModeData::TutorialGameModeData() : tutorial(0)
{
}

void GameModePacket::TutorialGameModeData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    tutorial = packet->ints[0];
}

void GameModePacket::TutorialGameModeData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(tutorial);
}

GameModePacket::UpdateSeedGameModeData::UpdateSeedGameModeData() : seed(0), mapSizeMultiplier(1.0f)
{
}

GameModePacket::UpdateSeedGameModeData::UpdateSeedGameModeData(long long seed, float mapSizeMultiplier)
    : seed(seed),
      mapSizeMultiplier(mapSizeMultiplier)
{
}

void GameModePacket::UpdateSeedGameModeData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    seed = _fromHEXString<long long>(packet->strings[0]);
    mapSizeMultiplier = packet->floats[0];
}

void GameModePacket::UpdateSeedGameModeData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->strings.push_back(_toHexString<long long>(seed));
    packet->floats.push_back(mapSizeMultiplier);
}

long long GameModePacket::UpdateSeedGameModeData::getSeed() const
{
    return seed;
}

double GameModePacket::UpdateSeedGameModeData::getMapSizeMultiplier() const
{
    return mapSizeMultiplier;
}

GameModePacket::UpdateThemeGameModeData::UpdateThemeGameModeData() : themeData(-1)
{
}

GameModePacket::UpdateThemeGameModeData::UpdateThemeGameModeData(int themeData) : themeData(themeData)
{
}

void GameModePacket::UpdateThemeGameModeData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    themeData = packet->ints[0];
}

void GameModePacket::UpdateThemeGameModeData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(themeData);
}

unsigned int GameModePacket::UpdateThemeGameModeData::getThemeData() const
{
    return themeData;
}

GameModePacket::SoundPositionData::SoundPositionData()
{
    x = y = z = 0.0f;
}

GameModePacket::SoundPositionData::SoundPositionData(float x, float y, float z) : x(x), y(y), z(z)
{
}

void GameModePacket::SoundPositionData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    x = packet->floats[0];
    y = packet->floats[1];
    z = packet->floats[2];
}

void GameModePacket::SoundPositionData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->floats.push_back(x);
    packet->floats.push_back(y);
    packet->floats.push_back(z);
}

GameModePacket::UpdateThemeIdData::UpdateThemeIdData() : themeId(-1), previousThemeId(-1)
{
}

GameModePacket::UpdateThemeIdData::UpdateThemeIdData(int themeId, int previousThemeId)
    : themeId(themeId),
      previousThemeId(previousThemeId)
{
}

void GameModePacket::UpdateThemeIdData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    themeId = packet->ints[0];
    previousThemeId = packet->ints[1];
}

void GameModePacket::UpdateThemeIdData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->ints.push_back(themeId);
    packet->ints.push_back(previousThemeId);
}

GameModePacket::RoundStartGameModeData::RoundStartGameModeData() : roundId(L""), subRoundId(L"")
{
}

GameModePacket::RoundStartGameModeData::RoundStartGameModeData(const std::wstring& roundId,
                                                               const std::wstring& subRoundId)
    : roundId(roundId),
      subRoundId(subRoundId)
{
}

void GameModePacket::RoundStartGameModeData::ReadPacket(boost::shared_ptr<GameModePacket> packet)
{
    roundId = packet->strings[0];
    subRoundId = packet->strings[1];
}

void GameModePacket::RoundStartGameModeData::WritePacket(boost::shared_ptr<GameModePacket> packet) const
{
    packet->strings.push_back(roundId);
    packet->strings.push_back(subRoundId);
}

std::wstring GameModePacket::RoundStartGameModeData::GetRoundId() const
{
    return roundId;
}

std::wstring GameModePacket::RoundStartGameModeData::GetSubRoundId() const
{
    return subRoundId;
}

GameModePacket::GameModePacket()
{
}

GameModePacket::GameModePacket(EMessage message) : message(message)
{
}

int GameModePacket::getPacketId()
{
    return 210;
}

void GameModePacket::read(DataInputStream* input)
{
    message = static_cast<EMessage>(input->readByte());
    FlexiblePacket::read(input);
}

void GameModePacket::write(DataOutputStream* output)
{
    output->writeByte(static_cast<unsigned char>(message));
    FlexiblePacket::write(output);
}

void GameModePacket::handle(PacketListener* listener)
{
    listener->handleGameMode(shared_from_this());
}

int GameModePacket::getEstimatedSize()
{
    return FlexiblePacket::getEstimatedSize() + 1;
}

void GameModePacket::WritePacket(const Data& data)
{
    data.WritePacket(shared_from_this());
}

boost::shared_ptr<GameModePacket> GameModePacket::Create(EMessage message, const Data& data)
{
    boost::shared_ptr<GameModePacket> packet(new GameModePacket(message));
    packet->WritePacket(data);
    return packet;
}

void GameModePacket::ReadPacket(Data& data)
{
    data.ReadPacket(shared_from_this());
}

unsigned int GameModePacket::GetMessage()
{
    return message;
}
