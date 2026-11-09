#ifndef GAME_WORLD_GENERATION_IQNETPLAYER_HPP
#define GAME_WORLD_GENERATION_IQNETPLAYER_HPP

#include "game/world/generation/PlayerUID.hpp"

class IQNetPlayer
{
public:
    unsigned char GetSmallId();

    void SendData(IQNetPlayer* destination, const void* data, unsigned int size, unsigned int channel);
    bool IsSameSystem(IQNetPlayer* other);
    unsigned int GetSendQueueSize(IQNetPlayer* destination, unsigned int channel);
    unsigned int GetCurrentRtt();

    bool IsHost();
    bool IsGuest();
    bool IsLocal();

    PlayerUID GetXuid();
    const wchar_t* GetGamertag();
    unsigned int GetSessionIndex();

    bool IsTalking();
    bool IsMutedByLocalUser(unsigned int userIndex);
    bool HasVoice();
    bool HasCamera();

    int GetUserIndex();

    void SetCustomDataValue(unsigned long customDataValue);
    unsigned long GetCustomDataValue();

private:
    unsigned long m_customDataValue;
};

#endif
