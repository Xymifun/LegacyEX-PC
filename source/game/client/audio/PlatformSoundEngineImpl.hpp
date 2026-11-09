#ifndef GAME_CLIENT_AUDIO_PLATFORMSOUNDENGINEIMPL_HPP
#define GAME_CLIENT_AUDIO_PLATFORMSOUNDENGINEIMPL_HPP

#include "game/client/audio/PlatformSoundEngine.hpp"

#ifdef MINECRAFT_PC
#include <vector>
#endif

struct _SAMPLE;
struct _STREAM;
struct _DIG_DRIVER;
struct SoundBank;

class PlatformSoundEngineImpl : public PlatformSoundEngine
{
public:
    PlatformSoundEngineImpl();
    virtual ~PlatformSoundEngineImpl();

    virtual void init(char* pSoundBankPath, SoundBankLoadedCallback pSoundBankLoaded);
    virtual void tick(void* pUserData, float fListenerX, float fListenerY, float fListenerZ, float fListenerForwardX,
                      float fListenerForwardY, float fListenerForwardZ, ConfigureSoundCallback pConfigureSound,
                      UpdateSoundCallback pUpdateSound, RouteToDRCCallback pRouteToDRC, bool* pbSystemMusicPlaying);
    virtual void startSound(void* pSoundData, int iSoundDataSize, char* pEventName);
    virtual bool openStream(char* pFileName);
    virtual void startStream(void* pUserData, ConfigureSoundCallback pConfigureSound);
    virtual void setStreamVol(float fVolume);
    virtual void stopStream();
    virtual void updateStreamPos(void* pUserData, UpdateSoundCallback pUpdateSound);
    virtual bool isStreamComplete();
    virtual void shutdown();
#ifdef MINECRAFT_PC
    virtual void startSampleFile(const void* pFile, bool bIs3D, float fMinDistance, float fPlaybackRate, float fX,
                                 float fY, float fZ, float fVolume);
#endif

private:
    static int initAX(void* pArg);
    static void ErrorCallback(long long iErrorCode, const char* pErrorText);
    static float custom_falloff_function(_SAMPLE* pSample, float fDistance, float fRolloffFactor, float fMinDistance,
                                         float fMaxDistance);

    SoundBank* m_pSoundBank;
    _DIG_DRIVER* m_pDigitalDriver;
    _STREAM* m_pStream;
    _STREAM* m_pStreamDRC;
#ifdef MINECRAFT_PC
    std::vector<_SAMPLE*> m_pcSamples;
#endif
};

#endif
