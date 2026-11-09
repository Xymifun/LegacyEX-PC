#ifndef GAME_CLIENT_AUDIO_SOUNDEVENT_HPP
#define GAME_CLIENT_AUDIO_SOUNDEVENT_HPP

#include <string>

class SoundEvent
{
public:
    enum EType
    {
        eType_Sound,
        eType_SFX,
        eType_Music
    };

    SoundEvent(std::wstring name, EType type, float distanceScalar);
    SoundEvent(std::wstring name, float distanceScalar);

    std::wstring getName() const;

    void setIndex(int index);
    int getIndex() const;

    float getDistanceScalar() const;

    bool isSFX() const;
    bool isMusic() const;

private:
    std::wstring name;
    int index;
    float distanceScalar;
    EType type;
};

#endif
