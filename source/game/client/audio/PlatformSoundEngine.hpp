#ifndef GAME_CLIENT_AUDIO_PLATFORMSOUNDENGINE_HPP
#define GAME_CLIENT_AUDIO_PLATFORMSOUNDENGINE_HPP

class PlatformSoundEngine
{
public:
    typedef void (*SoundBankLoadedCallback)();
    typedef void (*ConfigureSoundCallback)(void* userData, void* soundInstance, bool* is3D, float* minDistance,
                                           float* playbackRate, float* x, float* y, float* z, float* volume);
    typedef void (*UpdateSoundCallback)(void* userData, void* soundInstance, bool* is3D, int soundType, float* x,
                                        float* y, float* z, float* volume);
    typedef bool (*RouteToDRCCallback)(void* userData, void* soundInstance);

    virtual ~PlatformSoundEngine() {}

    virtual void init(char* soundBankPath, SoundBankLoadedCallback soundBankLoaded) = 0;
    virtual void tick(void* userData, float listenerX, float listenerY, float listenerZ, float listenerForwardX,
                      float listenerForwardY, float listenerForwardZ, ConfigureSoundCallback configureSound,
                      UpdateSoundCallback updateSound, RouteToDRCCallback routeToDRC, bool* systemMusicPlaying) = 0;
    virtual void startSound(void* soundData, int soundDataSize, char* eventName) = 0;
    virtual bool openStream(char* fileName) = 0;
    virtual void startStream(void* userData, ConfigureSoundCallback configureSound) = 0;
    virtual void setStreamVol(float volume) = 0;
    virtual void stopStream() = 0;
    virtual void updateStreamPos(void* userData, UpdateSoundCallback updateSound) = 0;
    virtual bool isStreamComplete() = 0;
    virtual void shutdown() = 0;
#ifdef MINECRAFT_PC
    virtual void startSampleFile(const void* file, bool is3D, float minDistance, float playbackRate, float x, float y,
                                 float z, float volume)
    {
    }
#endif
};

#endif
