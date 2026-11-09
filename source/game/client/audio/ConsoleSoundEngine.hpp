#ifndef GAME_CLIENT_AUDIO_CONSOLESOUNDENGINE_HPP
#define GAME_CLIENT_AUDIO_CONSOLESOUNDENGINE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/client/audio/AbstractSoundInstance.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/TickableSoundInstance.hpp"
#include "game/core/CriticalSection.hpp"

class File;
class Mob;
class Options;

class ConsoleSoundEngine
{
public:
    enum eAudioDelayType
    {
        eAudioDelayType_None,
        eAudioDelayType_Random,
        eAudioDelayType_Short,
        eAudioDelayType_ShortMenu,
        eAudioDelayType_ShortLobby,
        eAudioDelayType_ShortRound,
        eAudioDelayType_Unchanged
    };

    ConsoleSoundEngine();

    virtual ~ConsoleSoundEngine();

    virtual void tick(boost::shared_ptr<Mob>* listeners, float partialTicks) = 0;
    virtual void destroy() = 0;
    virtual void shutdown() = 0;
    virtual void play(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                      SoundInstance* instance) = 0;
    virtual void playStreaming(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                               bool checkPosition, bool waitForMusicStartEvent, eAudioDelayType delayType) = 0;
    virtual void playUI(const SoundEvent* sound, float volume, float pitch) = 0;
    virtual void init(Options* options) = 0;
    virtual void add(const std::wstring& name, File* file) = 0;
    virtual void addMusic(const std::wstring& name, File* file) = 0;
    virtual void addStreaming(const std::wstring& name, File* file) = 0;
    virtual char* ConvertSoundPathToName(const std::wstring& path, bool replaceSpaces) = 0;
    virtual void playMusicTick() = 0;

    virtual bool GetIsPlayingStreamingCDMusic();
    virtual bool GetIsPlayingStreamingGameMusic();
    virtual void SetIsPlayingStreamingCDMusic(bool playing);
    virtual void SetIsPlayingStreamingGameMusic(bool playing);
    virtual bool GetIsPlayingEndMusic();
    virtual bool GetIsPlayingNetherMusic();
    virtual void SetIsPlayingEndMusic(bool playing);
    virtual void SetIsPlayingNetherMusic(bool playing);

    void updateSystemMusicPlaying(bool playing);
    float getMasterMusicVolume();
    void setMasterMusicVolume(float volume);
    float getMasterSoundEffectVolume();
    void setMasterSoundEffectVolume(float volume);
    void LimitMusicVolume();
    void RemoveMusicVolumeLimit();

    void playInstance(SoundInstance* sound);
    void play(SoundInstance* sound);
    bool isPlaying(SoundInstance* sound);
    void stop(SoundInstance* sound);
    void stopAll();
    void tick();
    void schedule(const SoundEvent* sound, SoundSource::ESoundSource source, float x, float y, float z, float volume,
                  float pitch, int delay);

private:
    class ScheduledSound : public AbstractSoundInstance
    {
    public:
        ScheduledSound(const SoundEvent* sound, SoundSource::ESoundSource source, float x, float y, float z,
                       float volume, float pitch, int delay);
        ScheduledSound(const ScheduledSound& other);

        virtual ~ScheduledSound();
        virtual SoundInstance* getDRCClone() const;
    };

protected:
    long long playTime;
    OSFastMutex criticalSection;
    boost::unordered_map<SoundInstance*, SoundInstance*> playingSounds;
    std::vector<TickableSoundInstance*> tickableSounds;
    boost::unordered_map<SoundInstance*, long long> delayedSounds;
    boost::unordered_map<SoundInstance*, long long> playingSoundsStopTime;
    bool isPlayingStreamingCDMusic;
    bool isPlayingStreamingGameMusic;
    bool isPlayingEndMusic;
    bool isPlayingNetherMusic;
    int systemMusicPlaying;
    float masterMusicVolume;
    float masterSoundEffectVolume;
    bool musicVolumeLimited;
    bool musicVolumeFading;
    bool musicVolumeFadingUp;
    unsigned int unlimitedMusicVolume;
    unsigned int targetMusicVolume;
    unsigned int musicVolumeFadeStep;
};

#endif
