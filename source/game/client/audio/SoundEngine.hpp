#ifndef GAME_CLIENT_AUDIO_SOUNDENGINE_HPP
#define GAME_CLIENT_AUDIO_SOUNDENGINE_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/client/audio/ConsoleSoundEngine.hpp"
class C4JThreadImpl;
class File;
class Mob;
class Options;
class Random;
class SoundEvent;
class SoundInstance;
class PlatformSoundEngine;
struct AudioInfo
{
    float x;
    float y;
    float z;
    float volume;
    float pitch;
    const SoundEvent* soundEvent;
    bool is3D;
    bool isUI;
    bool routeToDRC;
    SoundInstance* instance;
};
struct ListenerInfo
{
    bool valid;
    float x;
    float y;
    float z;
    float forwardX;
    float forwardY;
    float forwardZ;
};
struct SoundTrack
{
    int sortKey;
    int trackIndex;
};
class SoundEngine : public ConsoleSoundEngine
{
public:
    enum eMusicType
    {
        eMusicType_Nether,
        eMusicType_1,
        eMusicType_Menu,
        eMusicType_3,
        eMusicType_End,
        eMusicType_Creative,
        eMusicType_Battle,
        eMusicType_Tumble,
        eMusicType_Glide,
        eMusicType_Build,
        eMusicType_Overworld
    };
    enum eMusicStreamState
    {
        eMusicStreamState_Idle,
        eMusicStreamState_Stop,
        eMusicStreamState_Stopping,
        eMusicStreamState_Opening,
        eMusicStreamState_OpeningCancel,
        eMusicStreamState_Play,
        eMusicStreamState_Playing,
        eMusicStreamState_Completed
    };
    enum
    {
        STREAM_FIRST_CD = 87,
        STREAM_COUNT = 99,
        MAX_LISTENERS = 4
    };
    SoundEngine();
    static int TracksSortFunction(const void*, const void*);
    void BuildTrackList(int, int, bool);
    void SetStreamingSounds(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int,
                            int);
    int GetNextTrack(int, int);
    void SetMusicID(eMusicType);
    virtual void destroy();
    virtual void shutdown();
    virtual void play(const SoundEvent*, float, float, float, float, float, SoundInstance*);
    eMusicType GetMusicToPlay();
    void SetMusicID(const std::wstring&);
    virtual void playStreaming(const SoundEvent*, float, float, float, float, float, bool, bool,
                               ConsoleSoundEngine::eAudioDelayType);
    virtual void playUI(const SoundEvent*, float, float);
    void updateMusicVolume(float);
    static int OpenStreamThreadProc(void*);
    float getDistanceScaler(AudioInfo*);
    static void UpdateSound(void*, void*, bool*, int, float*, float*, float*, float*);
    static void ConfigureSound(void*, void*, bool*, float*, float*, float*, float*, float*, float*);
    void playMusicUpdate();
    virtual void playMusicTick();
    virtual void init(Options*);
    static bool RouteToDRC(void*, void*);
    void updateMiles();
    virtual void tick(boost::shared_ptr<Mob>*, float);
    virtual void add(const std::wstring&, File*);
    virtual void addMusic(const std::wstring&, File*);
    virtual void addStreaming(const std::wstring&, File*);
    virtual char* ConvertSoundPathToName(const std::wstring&, bool);
    void FlagMusicEventStart();
    void updateSoundEffectVolume(float);
    bool isStreamingWavebankReady();

    static char m_szMusicPath[8];
    static const char* m_szStreamFileA[STREAM_COUNT];

private:
    ListenerInfo m_listeners[MAX_LISTENERS];
    int m_validListenerCount;
    Random* m_random;
    int m_musicID;
    int m_musicDelay;
    bool m_waitingForMusicStartEvent;
    int m_streamState;
    int m_streamType;
    int m_fadeOutTicks;
    AudioInfo m_streamingAudioInfo;
    std::wstring m_unusedString;
    PlatformSoundEngine* m_platformSoundEngine;
    C4JThreadImpl* m_openStreamThread;
    char m_streamName[256];
    int m_overworldFirst;
    int m_overworldLast;
    int m_netherFirst;
    int m_netherLast;
    int m_endFirst;
    int m_endLast;
    int m_creativeFirst;
    int m_creativeLast;
    int m_menuFirst;
    int m_menuLast;
    int m_battleFirst;
    int m_battleLast;
    int m_tumbleFirst;
    int m_tumbleLast;
    int m_glideFirst;
    int m_glideLast;
    int m_buildFirst;
    int m_buildLast;
    int m_firstCDTrack;
    unsigned char* m_trackPlayed;
    SoundTrack* m_tracks;
    int m_lastTrack;
};
#endif
