#ifndef GAME_CLIENT_AUDIO_SOUNDSOURCE_HPP
#define GAME_CLIENT_AUDIO_SOUNDSOURCE_HPP

class SoundSource
{
public:
    enum ESoundSource
    {
        eSoundSource_Master,
        eSoundSource_Music,
        eSoundSource_Records,
        eSoundSource_Weather,
        eSoundSource_Blocks,
        eSoundSource_Hostile,
        eSoundSource_Neutral,
        eSoundSource_Players,
        eSoundSource_Ambient,
        eSoundSource_Voice
    };
};

#endif
