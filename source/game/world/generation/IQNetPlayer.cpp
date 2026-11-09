#include "game/world/generation/IQNetPlayer.hpp"

static const int MAX_LOCAL_PLAYERS = 4;

static IQNetPlayer s_localPlayers[MAX_LOCAL_PLAYERS];

static const wchar_t* s_stubGamertag = L"stub";

unsigned char IQNetPlayer::GetSmallId()
{
    return 0;
}

void IQNetPlayer::SendData(IQNetPlayer* destination, const void* data, unsigned int size, unsigned int channel)
{
}

bool IQNetPlayer::IsSameSystem(IQNetPlayer* other)
{
    return true;
}

unsigned int IQNetPlayer::GetSendQueueSize(IQNetPlayer* destination, unsigned int channel)
{
    return 0;
}

unsigned int IQNetPlayer::GetCurrentRtt()
{
    return 0;
}

bool IQNetPlayer::IsHost()
{
    return this == &s_localPlayers[0];
}

bool IQNetPlayer::IsGuest()
{
    return false;
}

bool IQNetPlayer::IsLocal()
{
    return true;
}

PlayerUID IQNetPlayer::GetXuid()
{
    return PlayerUID(0);
}

const wchar_t* IQNetPlayer::GetGamertag()
{
    return s_stubGamertag;
}

unsigned int IQNetPlayer::GetSessionIndex()
{
    return 0;
}

bool IQNetPlayer::IsTalking()
{
    return false;
}

bool IQNetPlayer::IsMutedByLocalUser(unsigned int userIndex)
{
    return false;
}

bool IQNetPlayer::HasVoice()
{
    return false;
}

bool IQNetPlayer::HasCamera()
{
    return false;
}

int IQNetPlayer::GetUserIndex()
{
    return this - s_localPlayers;
}

void IQNetPlayer::SetCustomDataValue(unsigned long customDataValue)
{
    m_customDataValue = customDataValue;
}

unsigned long IQNetPlayer::GetCustomDataValue()
{
    return m_customDataValue;
}
