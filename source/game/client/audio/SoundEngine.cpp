#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/DLCAudioFile.hpp"
#include "game/client/audio/PlatformSoundEngine.hpp"
#include "game/client/audio/PlatformSoundEngineImpl.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/DefaultTexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CStorage.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef MINECRAFT_PC
#include <vector>

#include "platform/pc/PcGoldenAge.hpp"
#endif

#ifdef MINECRAFT_PC
extern "C" const char* AIL_last_error();
namespace
{
int s_pcOpenFailures = 0;
const int kPcMaxOpenFailures = 3;
}
#endif

extern void SetSpecialThreadIDForFS_MilesStreamThread();

namespace
{

const int DIMENSION_NETHER = -1;
const int DIMENSION_END = 1;
const int STREAM_TYPE_GAME_MUSIC = 10;
const int STREAM_TYPE_CD = 2;
const int FADE_OUT_TICKS = 200;
const int MINIGAME_MUSIC_DELAY = 60;
const unsigned int SHORT_MUSIC_DELAY_TEXTURE_PACK_ID = 1036;
const float PI = 3.1415927f;

}

char SoundEngine::m_szMusicPath[8] = "music/";

const char* SoundEngine::m_szStreamFileA[SoundEngine::STREAM_COUNT] = {
    "calm1",
    "calm2",
    "calm3",
    "hal1",
    "hal2",
    "hal3",
    "hal4",
    "nuance1",
    "nuance2",
    "piano1",
    "piano2",
    "piano3",
    "nether1",
    "nether2",
    "nether3",
    "nether4",
    "the_end_dragon_alive",
    "the_end_end",
    "creative1",
    "creative2",
    "creative3",
    "creative4",
    "creative5",
    "creative6",
    "menu1",
    "menu2",
    "menu3",
    "menu4",
    "MG01_01",
    "MG01_02",
    "MG01_03",
    "MG01_04",
    "MG02_01",
    "MG02_02",
    "MG02_03",
    "MG02_04",
    "MG02_05",
    "MG02_06",
    "MG02_07",
    "MG02_08",
    "MG02_09",
    "MG02_10",
    "MG03_Vanilla_Map1Track1",
    "MG03_Vanilla_Map2Track1",
    "MG03_Vanilla_Map3Track1",
    "MG03_Vanilla_Map4Track1",
    "MG03_Vanilla_Map5Track1",
    "MG03_Vanilla_Map6Track1",
    "MG03_Vanilla_Map7Track1",
    "MG03_Vanilla_Pack2TrackBody1",
    "MG03_Vanilla_Pack2TrackBody2",
    "MG03_Vanilla_Pack2TrackBody3",
    "MG03_Vanilla_Canyon1",
    "MG03_Vanilla_Canyon2",
    "MG03_Vanilla_Canyon3",
    "MG03_Plastic_Pack2TrackShrunk1",
    "MG03_Plastic_Pack2TrackShrunk2",
    "MG03_Plastic_Pack2TrackShrunk3",
    "MG03_Vanilla_Pack3_Celts1",
    "MG03_Vanilla_Pack3_Celts2",
    "MG03_Fantasy_Pack3_Excalibur1",
    "MG03_Fantasy_Pack3_Excalibur2",
    "MG04_01",
    "creative1",
    "creative2",
    "creative3",
    "creative4",
    "creative5",
    "creative6",
    "Fantasy_BornToArms",
    "Fantasy_Crusaders",
    "Fantasy_Hastilude",
    "City_8BitsPiecesChunks",
    "City_BeatAroundTheBlock",
    "City_FromDistrictToDistrict",
    "City_IndustrialQuarter",
    "City_PrecinctsAtNight",
    "Steampunk_AirshipAdventurers",
    "Steampunk_ClockworkCrafter",
    "Steampunk_MagnificentMachines",
    "Steampunk_SteamingSuperstructures",
    "MG01_Plastic_Bedroom_ToysOnATear",
    "MG01_Plastic_Bedroom_DanceOTBlocks",
    "MG01_Plastic_Shrunk_MasterBuilders",
    "MG01_Vanilla_Western_DesertDuel",
    "MG01_Vanilla_Western_SunsetRiders",
    "MG01_Vanilla_Western_TheOutlaw",
    "11",
    "13",
    "blocks",
    "cat",
    "chirp",
    "far",
    "mall",
    "mellohi",
    "stal",
    "strad",
    "ward",
    "where_are_we_now",
};

SoundEngine::SoundEngine()
{
    m_random = new Random();
    m_streamState = eMusicStreamState_Idle;
    m_musicDelay = 0;
    m_waitingForMusicStartEvent = false;
    m_validListenerCount = 0;
    m_trackPlayed = NULL;
    m_tracks = NULL;
    SetStreamingSounds(0, 11, 12, 15, 16, 17, 18, 23, 24, 27, 28, 31, 32, 41, 42, 42, 62, 68, STREAM_FIRST_CD);
    SetMusicID(eMusicType_Menu);
    m_streamingAudioInfo.volume = 1.0f;
    m_fadeOutTicks = 0;
    m_streamingAudioInfo.is3D = false;
    m_streamingAudioInfo.pitch = 1.0f;
    m_streamingAudioInfo.instance = NULL;
    m_streamingAudioInfo.y = 0.0f;
    m_streamingAudioInfo.x = 0.0f;
    m_streamingAudioInfo.z = 0.0f;
    std::memset(m_listeners, 0, sizeof(m_listeners));
    m_platformSoundEngine = new PlatformSoundEngineImpl();
}

int SoundEngine::TracksSortFunction(const void* left, const void* right)
{
    return static_cast<const SoundTrack*>(left)->sortKey - static_cast<const SoundTrack*>(right)->sortKey;
}

void SoundEngine::BuildTrackList(int first, int last, bool keepFirstTrack)
{
    for (int track = first; track <= last; ++track)
    {
        m_trackPlayed[track] = 0;
        m_tracks[track].sortKey = m_random->nextInt(100000);
        m_tracks[track].trackIndex = track;
    }

    if (first == last)
        return;

    if (keepFirstTrack)
    {
        std::qsort(m_tracks + first + 1, last - first, sizeof(SoundTrack), TracksSortFunction);
        return;
    }

    std::qsort(m_tracks + first, (last - first) + 1, sizeof(SoundTrack), TracksSortFunction);

    if (m_lastTrack != -1 && m_tracks[first].trackIndex == m_lastTrack)
    {
        const int displaced = m_tracks[first + 1].trackIndex;
        m_tracks[first + 1].trackIndex = m_tracks[first].trackIndex;
        m_tracks[first].trackIndex = displaced;
    }
}

void SoundEngine::SetStreamingSounds(int overworldFirst, int overworldLast, int netherFirst, int netherLast,
                                     int endFirst, int endLast, int creativeFirst, int creativeLast, int menuFirst,
                                     int menuLast, int battleFirst, int battleLast, int tumbleFirst, int tumbleLast,
                                     int glideFirst, int glideLast, int buildFirst, int buildLast, int firstCDTrack)
{
    m_overworldFirst = overworldFirst;
    m_overworldLast = overworldLast;
    m_netherFirst = netherFirst;
    m_netherLast = netherLast;
    m_endFirst = endFirst;
    m_endLast = endLast;
    m_creativeFirst = creativeFirst;
    m_creativeLast = creativeLast;
    m_menuFirst = menuFirst;
    m_menuLast = menuLast;
    m_battleFirst = battleFirst;
    m_battleLast = battleLast;
    m_tumbleFirst = tumbleFirst;
    m_tumbleLast = tumbleLast;
    m_glideFirst = glideFirst;
    m_glideLast = glideLast;
    m_buildFirst = buildFirst;
    m_buildLast = buildLast;
    m_firstCDTrack = firstCDTrack;

    delete[] m_trackPlayed;
    m_trackPlayed = new unsigned char[firstCDTrack];
    std::memset(m_trackPlayed, 0, firstCDTrack);

    delete[] m_tracks;
    m_tracks = new SoundTrack[firstCDTrack];
    std::memset(m_tracks, 0, (firstCDTrack - 1) * sizeof(SoundTrack) + 1);

    m_lastTrack = -1;

    BuildTrackList(overworldFirst, overworldLast, false);
    BuildTrackList(netherFirst, netherLast, false);
    BuildTrackList(endFirst, endLast, false);
    BuildTrackList(creativeFirst, creativeLast, false);
    BuildTrackList(menuFirst, menuLast, false);
    BuildTrackList(battleFirst, battleLast, false);
    BuildTrackList(tumbleFirst, tumbleLast, false);
    BuildTrackList(glideFirst, glideLast, false);
    BuildTrackList(buildFirst, buildLast, true);
}

int SoundEngine::GetNextTrack(int first, int last)
{
    unsigned int playedCount = 0;
    unsigned int rangeSize = (last - first) + 1;
    int chosen = m_tracks[first].trackIndex;
    bool picked = false;

    for (int track = last; track >= first; --track)
    {
        if (m_trackPlayed[track] == 0)
        {
            if (!picked)
            {
                picked = true;
                chosen = m_tracks[track].trackIndex;
                m_trackPlayed[track] = 1;
            }
        }
        else
        {
            ++playedCount;
        }
    }

    if (playedCount == 0 || playedCount == rangeSize)
    {
        BuildTrackList(first, last, false);
        chosen = m_tracks[first].trackIndex;
        m_trackPlayed[first] = 1;
    }

    m_lastTrack = m_tracks[chosen].trackIndex;
    return m_lastTrack;
}

void SoundEngine::SetMusicID(eMusicType type)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool defaultPack = true;
    if (minecraft != NULL)
    {
        defaultPack = minecraft->mTexturePackRepository->getSelected() == TexturePackRepository::defaultTexturePack;
    }

    if (defaultPack)
    {
        switch (type)
        {
        case eMusicType_Nether:
            m_musicID = GetNextTrack(m_netherFirst, m_netherLast);
            return;
        case eMusicType_Menu:
            m_musicID = GetNextTrack(m_menuFirst, m_menuLast);
            return;
        case eMusicType_End:
        {
            Minecraft* instance = Minecraft::GetInstance();
            BossHealthOverlay* overlay = instance->mGui->getBossOverlay(instance->mPlayer->mUserIndex);
            if (!overlay->shouldPlayMusic())
                m_musicID = m_endLast;
            else
                m_musicID = m_endFirst;
            return;
        }
        case eMusicType_Creative:
            m_musicID = GetNextTrack(m_creativeFirst, m_creativeLast);
            return;
        case eMusicType_Battle:
            m_musicID = GetNextTrack(m_battleFirst, m_battleLast);
            return;
        case eMusicType_Tumble:
            m_musicID = GetNextTrack(m_tumbleFirst, m_tumbleLast);
            return;
        case eMusicType_Glide:
            m_musicID = GetNextTrack(m_glideFirst, m_glideLast);
            return;
        case eMusicType_Build:
            m_musicID = GetNextTrack(m_buildFirst, m_buildLast);
            return;
        default:
            break;
        }
    }
    else
    {
        switch (type)
        {
        case eMusicType_Nether:
            m_musicID = GetNextTrack(m_netherFirst, m_netherLast);
            return;
        case eMusicType_Menu:
            m_musicID = GetNextTrack(m_menuFirst, m_menuLast);
            return;
        case eMusicType_End:
            m_musicID = GetNextTrack(m_endFirst, m_endLast);
            return;
        case eMusicType_Creative:
            m_musicID = GetNextTrack(m_creativeFirst, m_creativeLast);
            return;
        case eMusicType_Battle:
            m_musicID = GetNextTrack(m_battleFirst, m_battleLast);
            return;
        case eMusicType_Tumble:
            m_musicID = GetNextTrack(m_tumbleFirst, m_tumbleLast);
            return;
        case eMusicType_Glide:
            m_musicID = GetNextTrack(m_glideFirst, m_glideLast);
            return;
        case eMusicType_Build:
            m_musicID = GetNextTrack(m_buildFirst, m_buildLast);
            return;
        default:
            break;
        }
    }

    m_musicID = GetNextTrack(m_overworldFirst, m_overworldLast);
}

void SoundEngine::destroy()
{
}

void SoundEngine::shutdown()
{
    m_platformSoundEngine->shutdown();
}

void SoundEngine::play(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                       SoundInstance* instance)
{
    if (sound == NULL)
        return;

    char eventName[256];
    std::strcpy(eventName, "Minecraft/");
    std::wstring soundName = sound->getName();
#ifdef MINECRAFT_PC
    const char* goldenAgeEvent = PcGoldenAgeEventName(sound);
    if (goldenAgeEvent != NULL)
        std::strcat(eventName, goldenAgeEvent);
    else
#endif
        std::strcat(eventName, ConvertSoundPathToName(soundName, false));

    AudioInfo info;
    info.x = x;
    info.y = y;
    info.z = z;
    info.volume = volume;
    info.pitch = pitch;
    info.soundEvent = sound;
    info.is3D = true;
    info.isUI = false;
    info.instance = instance;

#ifdef MINECRAFT_PC
    const std::vector<char>* goldenAgeFile = instance == NULL ? PcGoldenAgeSoundFile(sound) : NULL;
    if (goldenAgeFile != NULL)
    {
        info.routeToDRC = false;
        bool is3D = true;
        float minDistance = 16.0f;
        float playbackRate = 1.0f;
        float soundX = x;
        float soundY = y;
        float soundZ = z;
        float soundVolume = volume;
        ConfigureSound(this, &info, &is3D, &minDistance, &playbackRate, &soundX, &soundY, &soundZ, &soundVolume);
        m_platformSoundEngine->startSampleFile(&(*goldenAgeFile)[0], is3D, minDistance, playbackRate, soundX, soundY,
                                               soundZ, soundVolume);
        return;
    }
#endif

    if (instance != NULL)
    {
        instance->setSIState(SoundInstance::eSoundState_Pending);
        info.routeToDRC = instance->drcClone;
        instance->pendingTicks = 0;
        m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
    }
    else
    {
        info.routeToDRC = true;
        m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
        info.routeToDRC = false;
        m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
    }
}

SoundEngine::eMusicType SoundEngine::GetMusicToPlay()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    eMusicType musicType;
    bool playerInEnd = false;
    bool playerInNether = false;
    bool noPlayers = true;
    int lastPlayer = 0;

    for (int i = 0; i < MAX_LISTENERS; ++i)
    {
        if (minecraft->mLocalPlayers[i] != NULL)
        {
            noPlayers = false;
            lastPlayer = i;
            if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_END)
                playerInEnd = true;
            else if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_NETHER)
                playerInNether = true;
        }
    }

    if (playerInEnd)
    {
        musicType = eMusicType_End;
    }
    else if (playerInNether)
    {
        musicType = eMusicType_Nether;
    }
    else if (noPlayers)
    {
        if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
            musicType = eMusicType_Battle;
        else
            musicType = eMusicType_Menu;
    }
    else
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
        if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
        {
            musicType = eMusicType_Creative;
        }
        else
        {
            switch (minecraft->GetMiniGame()->GetId())
            {
            case eMiniGameId_Battle:
                musicType = eMusicType_Battle;
                break;
            case eMiniGameId_Tumble:
                musicType = eMusicType_Tumble;
                break;
            case eMiniGameId_Glide:
                musicType = eMusicType_Glide;
                break;
            case eMiniGameId_Build:
                musicType = eMusicType_Build;
                break;
            default:
                musicType = eMusicType_Overworld;
                break;
            }
        }
    }
    return musicType;
}

void SoundEngine::SetMusicID(const std::wstring& name)
{
    int musicID = 0;
    char* soundName = ConvertSoundPathToName(name, true);

    for (int i = 0; i < STREAM_COUNT - STREAM_FIRST_CD; ++i)
    {
        if (std::strcmp(soundName, m_szStreamFileA[STREAM_FIRST_CD + i]) == 0)
        {
            musicID = i;
            break;
        }
    }

    m_musicID = musicID + m_firstCDTrack;
}

void SoundEngine::playStreaming(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                                bool checkPosition, bool waitForMusicStartEvent,
                                ConsoleSoundEngine::eAudioDelayType delayType)
{
    if (sound == NULL && checkPosition && m_streamingAudioInfo.is3D
        && (m_streamingAudioInfo.x != x || m_streamingAudioInfo.y != y || m_streamingAudioInfo.z != z))
        return;

    m_streamingAudioInfo.x = x;
    m_streamingAudioInfo.y = y;
    m_streamingAudioInfo.z = z;
    m_streamingAudioInfo.volume = volume;
    m_streamingAudioInfo.pitch = pitch;

    if (m_streamState == eMusicStreamState_Playing)
    {
        m_fadeOutTicks = FADE_OUT_TICKS;
        m_streamState = eMusicStreamState_Stop;
    }
    else if (m_streamState == eMusicStreamState_Opening)
    {
        m_streamState = eMusicStreamState_OpeningCancel;
    }

    if (sound == NULL)
    {
        m_streamingAudioInfo.is3D = false;
        switch (delayType)
        {
        case ConsoleSoundEngine::eAudioDelayType_None:
            m_musicDelay = 0;
            break;
        case ConsoleSoundEngine::eAudioDelayType_Random:
            m_musicDelay = m_random->nextInt(3600) + 600;
            break;
        case ConsoleSoundEngine::eAudioDelayType_Short:
        case ConsoleSoundEngine::eAudioDelayType_ShortMenu:
        case ConsoleSoundEngine::eAudioDelayType_ShortLobby:
        case ConsoleSoundEngine::eAudioDelayType_ShortRound:
            m_musicDelay = 60;
            break;
        default:
            break;
        }

        m_waitingForMusicStartEvent = waitForMusicStartEvent;
#ifdef MINECRAFT_PC
        if (waitForMusicStartEvent)
        {
            std::printf("[pc] music: waiting for the round to start\n");
            std::fflush(stdout);
        }
#endif
        SetMusicID(GetMusicToPlay());
    }
    else
    {
        m_streamingAudioInfo.is3D = true;
        SetMusicID(sound->getName());
        m_musicDelay = 0;
    }
}

void SoundEngine::playUI(const SoundEvent* sound, float volume, float pitch)
{
    char eventName[256];
    std::wstring soundName;
    if (!sound->isSFX())
        std::strcpy(eventName, "Minecraft/");
    else
        std::strcpy(eventName, "Minecraft/UI/");
    soundName = sound->getName();
    std::strcat(eventName, ConvertSoundPathToName(soundName, false));

    AudioInfo info;
    std::memset(&info, 0, sizeof(AudioInfo));
    info.volume = volume;
    info.pitch = pitch;
    info.soundEvent = sound;
    info.isUI = true;
    info.routeToDRC = true;
    m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
    info.routeToDRC = false;
    m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
}

void SoundEngine::updateMusicVolume(float volume)
{
    setMasterMusicVolume(volume);
}

int SoundEngine::OpenStreamThreadProc(void* param)
{
    SetSpecialThreadIDForFS_MilesStreamThread();
    SoundEngine* engine = static_cast<SoundEngine*>(param);
    if (engine->m_platformSoundEngine->openStream(engine->m_streamName))
        return 0;
    return -1;
}

float SoundEngine::getDistanceScaler(AudioInfo* info)
{
    float scaler = info->soundEvent->getDistanceScalar();
    if (info->volume == 10000.0f)
        scaler = 10000.0f;
    return scaler;
}

void SoundEngine::UpdateSound(void* userData, void* soundData, bool* is3D, int soundState, float* x, float* y, float* z,
                              float* volume)
{
    SoundEngine* engine = static_cast<SoundEngine*>(userData);
    AudioInfo* info = static_cast<AudioInfo*>(soundData);
    bool isStream = false;
    if (info == NULL)
    {
        isStream = true;
        info = &engine->m_streamingAudioInfo;
    }

    if (info->instance != NULL)
    {
        switch (soundState)
        {
        case 0:
            info->instance->setSIState(SoundInstance::eSoundState_Starting);
            break;
        case 2:
            info->instance->setSIState(SoundInstance::eSoundState_Complete);
            break;
        case 1:
            info->instance->setSIState(SoundInstance::eSoundState_Playing);
            info->x = info->instance->getX();
            info->y = info->instance->getY();
            info->z = info->instance->getZ();
            info->volume = info->instance->getVolume();
            info->pitch = info->instance->getPitch();
            break;
        }
    }

    if (info->volume > 1.0f)
        info->volume = 1.0f;

    if (isStream)
        *volume = info->volume * engine->getMasterMusicVolume();
    else
        *volume = info->volume * engine->getMasterSoundEffectVolume();

    *is3D = info->is3D;
    if (info->is3D)
    {
        if (engine->m_validListenerCount > 1)
        {
            float nearest = 10000.0f;
            float nearestX = 0.0f;
            float nearestY = 0.0f;
            float nearestZ = 0.0f;
            for (int i = 0; i < MAX_LISTENERS; ++i)
            {
                if (engine->m_listeners[i].valid)
                {
                    float dx = fabs(engine->m_listeners[i].x - info->x);
                    float dy = fabs(engine->m_listeners[i].y - info->y);
                    float dz = fabs(engine->m_listeners[i].z - info->z);
                    float distance = dx + dy + dz;
                    if (distance < nearest)
                    {
                        nearest = distance;
                        nearestX = dx;
                        nearestY = dy;
                        nearestZ = dz;
                    }
                }
            }

            float distance = sqrtf(nearestX * nearestX + nearestY * nearestY + nearestZ * nearestZ);
            *x = 0.0f;
            *y = 0.0f;
            *z = distance;
        }
        else
        {
            *x = info->x;
            *y = info->y;
            *z = -info->z;
        }
    }
}

void SoundEngine::ConfigureSound(void* userData, void* soundData, bool* is3D, float* minDistance, float* pitch,
                                 float* x, float* y, float* z, float* volume)
{
    SoundEngine* engine = static_cast<SoundEngine*>(userData);

    if (soundData != NULL)
    {
        *minDistance = engine->getDistanceScaler(static_cast<AudioInfo*>(soundData));
        AudioInfo* info = static_cast<AudioInfo*>(soundData);
        if (!info->isUI)
        {
            *pitch = info->pitch;
            UpdateSound(engine, info, is3D, 0, x, y, z, volume);
        }
        else
        {
            *pitch = 0.0f;
            UpdateSound(engine, info, is3D, 0, x, y, z, volume);
        }
    }
    else
    {
        *minDistance = 64.0f;
        *pitch = engine->m_streamingAudioInfo.pitch;
        UpdateSound(engine, NULL, is3D, 0, x, y, z, volume);
    }
}

void SoundEngine::playMusicUpdate()
{
    static float s_musicVolume;
    static bool s_firstUpdate = true;

    if (s_firstUpdate)
    {
        s_musicVolume = getMasterMusicVolume();
        s_firstUpdate = false;
    }

    if (musicVolumeFading)
    {
        const unsigned int current = static_cast<unsigned int>(getMasterMusicVolume() * 100.0f);
        const unsigned int target = targetMusicVolume;
        if (musicVolumeFadingUp)
        {
            if (current >= target)
            {
                musicVolumeFading = false;
                if (current > target)
                    updateMusicVolume(static_cast<float>(target) / 100.0f);
                musicVolumeLimited = !musicVolumeLimited;
            }
            else
            {
                updateMusicVolume(static_cast<float>(current + musicVolumeFadeStep) / 100.0f);
            }
        }
        else
        {
            if (current <= target)
            {
                musicVolumeFading = false;
                if (current < target)
                    updateMusicVolume(static_cast<float>(target) / 100.0f);
                musicVolumeLimited = !musicVolumeLimited;
            }
            else
            {
                updateMusicVolume(static_cast<float>(current - musicVolumeFadeStep) / 100.0f);
            }
        }
    }

    switch (m_streamState)
    {
    case eMusicStreamState_Idle:
        if (m_waitingForMusicStartEvent)
            return;

        if (m_musicDelay > 0)
        {
            --m_musicDelay;
            return;
        }

        if (m_musicID != -1)
        {
            std::sprintf(m_streamName, "%s", m_szMusicPath);

            if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()->hasAudio())
            {
                DLCTexturePack* pack = static_cast<DLCTexturePack*>(
                    Minecraft::GetInstance()->mTexturePackRepository->getSelected());
                DLCAudioFile* audioFile = static_cast<DLCAudioFile*>(
                    pack->getDLCInfoParentPack()->getFile(DLCManager::eDLCType_Audio, 0, true, true));

                if (m_musicID < m_firstCDTrack)
                {
                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_streamType = STREAM_TYPE_GAME_MUSIC;
                    m_streamingAudioInfo.is3D = false;

                    char fileName[255];
                    std::wcstombs(fileName, audioFile->GetSoundName(m_musicID).c_str(), 255);
                    std::string streamPath = "TPACK:\\Data\\" + std::string(fileName) + ".binka";
                    std::string mountedPath = gStorage->GetMountedPath(streamPath);
                    std::strcpy(m_streamName, mountedPath.c_str());
                }
                else
                {
                    SetIsPlayingStreamingGameMusic(false);
                    SetIsPlayingStreamingCDMusic(true);
                    const int stream = m_musicID - m_firstCDTrack + STREAM_FIRST_CD;
                    m_streamType = STREAM_TYPE_CD;
                    m_streamingAudioInfo.is3D = true;
                    if (stream >= STREAM_COUNT)
                        return;

                    std::strcat(m_streamName, "cds/");
                    std::strcat(m_streamName, m_szStreamFileA[stream]);
                    std::strcat(m_streamName, ".binka");
                }
            }
            else
            {
                if (m_musicID < m_firstCDTrack)
                {
                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_streamingAudioInfo.is3D = false;
                    m_streamType = STREAM_TYPE_GAME_MUSIC;
                    std::strcat(m_streamName, "music/");
                }
                else
                {
                    SetIsPlayingStreamingGameMusic(false);
                    SetIsPlayingStreamingCDMusic(true);
                    m_streamType = STREAM_TYPE_CD;
                    m_streamingAudioInfo.is3D = true;
                    std::strcat(m_streamName, "cds/");
                }
                std::strcat(m_streamName, m_szStreamFileA[m_musicID]);
                std::strcat(m_streamName, ".binka");
            }

#ifdef MINECRAFT_PC
            std::printf("[pc] music: opening %s (track %d)\n", m_streamName, m_musicID);
            std::fflush(stdout);
#endif
            m_openStreamThread = new C4JThreadImpl(OpenStreamThreadProc, this, "OpenStreamThreadProc", 0);
            m_openStreamThread->SetProcessor(2);
            m_openStreamThread->Run();
            m_streamState = eMusicStreamState_Opening;
        }
        break;

    case eMusicStreamState_Opening:
        if (!m_openStreamThread->isRunning())
        {
            const int result = m_openStreamThread->GetExitCode();
            delete m_openStreamThread;
#ifdef MINECRAFT_PC
            if (result != 0 && ++s_pcOpenFailures >= kPcMaxOpenFailures)
            {
                std::printf("[pc] music: cannot open %s (%s); going on to the next track\n", m_streamName,
                            AIL_last_error());
                std::fflush(stdout);
                s_pcOpenFailures = 0;
                m_openStreamThread = NULL;
                SetIsPlayingStreamingCDMusic(false);
                SetIsPlayingStreamingGameMusic(false);
                m_streamState = eMusicStreamState_Completed;
                break;
            }
            if (result == 0)
            {
                s_pcOpenFailures = 0;
                std::printf("[pc] music: playing %s\n", m_streamName);
                std::fflush(stdout);
            }
#endif
            if (result != 0)
            {
                m_openStreamThread = new C4JThreadImpl(OpenStreamThreadProc, this, "RetryOpenStreamThreadProc", 0);
                m_openStreamThread->SetProcessor(2);
                m_openStreamThread->Run();
            }
            else
            {
                m_openStreamThread = NULL;
                m_platformSoundEngine->startStream(this, ConfigureSound);
                m_streamState = eMusicStreamState_Playing;
            }
        }
        break;

    case eMusicStreamState_OpeningCancel:
        if (!m_openStreamThread->isRunning())
        {
            delete m_openStreamThread;
            m_streamState = eMusicStreamState_Stop;
            m_openStreamThread = NULL;
        }
        break;

    case eMusicStreamState_Stop:
        if (m_fadeOutTicks != 0)
        {
            --m_fadeOutTicks;
            const float fade = static_cast<float>(m_fadeOutTicks) / 200.0f;
            m_platformSoundEngine->setStreamVol(m_streamingAudioInfo.volume * getMasterMusicVolume() * fade);
        }
        else
        {
            m_platformSoundEngine->stopStream();
            SetIsPlayingStreamingCDMusic(false);
            SetIsPlayingStreamingGameMusic(false);
            m_streamState = eMusicStreamState_Idle;
        }
        break;

    case eMusicStreamState_Playing:
        if (GetIsPlayingStreamingGameMusic())
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            bool playerInEnd = false;
            bool playerInNether = false;
            bool noPlayers = true;
            int lastPlayer = 0;

            for (int i = 0; i < MAX_LISTENERS; ++i)
            {
                if (minecraft->mLocalPlayers[i] != NULL)
                {
                    noPlayers = false;
                    lastPlayer = i;
                    if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_END)
                        playerInEnd = true;
                    else if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_NETHER)
                        playerInNether = true;
                }
            }

            if (playerInEnd && !GetIsPlayingEndMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                SetMusicID(eMusicType_End);
                SetIsPlayingEndMusic(true);
                SetIsPlayingNetherMusic(false);
            }
            else if (!playerInEnd && GetIsPlayingEndMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                if (playerInNether)
                {
                    SetMusicID(eMusicType_Nether);
                    SetIsPlayingEndMusic(false);
                    SetIsPlayingNetherMusic(true);
                }
                else
                {
                    if (noPlayers)
                    {
                        SetMusicID(eMusicType_Menu);
                    }
                    else
                    {
                        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
                        if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
                        {
                            SetMusicID(eMusicType_Creative);
                        }
                        else
                        {
                            switch (minecraft->GetMiniGame()->GetId())
                            {
                            case eMiniGameId_Battle:
                                SetMusicID(eMusicType_Battle);
                                break;
                            case eMiniGameId_Tumble:
                                SetMusicID(eMusicType_Tumble);
                                break;
                            case eMiniGameId_Glide:
                                SetMusicID(eMusicType_Glide);
                                break;
                            case eMiniGameId_Build:
                                SetMusicID(eMusicType_Build);
                                break;
                            default:
                                SetMusicID(eMusicType_Overworld);
                                break;
                            }
                        }
                    }
                    SetIsPlayingEndMusic(false);
                    SetIsPlayingNetherMusic(false);
                }
            }
            else if (playerInNether && !GetIsPlayingNetherMusic() && !GetIsPlayingEndMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                SetMusicID(eMusicType_Nether);
                SetIsPlayingNetherMusic(true);
                SetIsPlayingEndMusic(false);
            }
            else if (!playerInNether && GetIsPlayingNetherMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                if (playerInEnd)
                {
                    SetMusicID(eMusicType_End);
                    SetIsPlayingNetherMusic(false);
                    SetIsPlayingEndMusic(true);
                }
                else
                {
                    if (noPlayers)
                    {
                        SetMusicID(eMusicType_Menu);
                    }
                    else
                    {
                        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
                        if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
                        {
                            SetMusicID(eMusicType_Creative);
                        }
                        else
                        {
                            switch (minecraft->GetMiniGame()->GetId())
                            {
                            case eMiniGameId_Battle:
                                SetMusicID(eMusicType_Battle);
                                break;
                            case eMiniGameId_Tumble:
                                SetMusicID(eMusicType_Tumble);
                                break;
                            case eMiniGameId_Glide:
                                SetMusicID(eMusicType_Glide);
                                break;
                            case eMiniGameId_Build:
                                SetMusicID(eMusicType_Build);
                                break;
                            default:
                                SetMusicID(eMusicType_Overworld);
                                break;
                            }
                        }
                    }
                    SetIsPlayingNetherMusic(false);
                    SetIsPlayingEndMusic(false);
                }
            }
        }
        else if (m_streamingAudioInfo.is3D)
        {
            m_platformSoundEngine->updateStreamPos(this, UpdateSound);
        }

        {
            float musicVolume = getMasterMusicVolume();
            if (s_musicVolume != musicVolume)
            {
                s_musicVolume = musicVolume;
                m_platformSoundEngine->setStreamVol(s_musicVolume);
            }
        }
        break;

    case eMusicStreamState_Completed:
    {
        m_musicDelay = m_random->nextInt(3600) + 600;

        Minecraft* minecraft = Minecraft::GetInstance();
        bool playerInEnd = false;
        bool playerInNether = false;
        bool noPlayers = true;
        int lastPlayer = 0;

        for (int i = 0; i < MAX_LISTENERS; ++i)
        {
            if (minecraft->mLocalPlayers[i] != NULL)
            {
                noPlayers = false;
                lastPlayer = i;
                if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_END)
                    playerInEnd = true;
                else if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_NETHER)
                    playerInNether = true;
            }
        }

        if (playerInEnd)
        {
            SetMusicID(eMusicType_End);
            SetIsPlayingEndMusic(true);
            SetIsPlayingNetherMusic(false);
        }
        else if (playerInNether)
        {
            SetMusicID(eMusicType_Nether);
            SetIsPlayingNetherMusic(true);
            SetIsPlayingEndMusic(false);
        }
        else
        {
            if (noPlayers)
            {
                if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getId()
                    == SHORT_MUSIC_DELAY_TEXTURE_PACK_ID)
                    SetMusicID(eMusicType_Overworld);
                else
                    SetMusicID(eMusicType_Menu);
            }
            else
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
                if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
                {
                    SetMusicID(eMusicType_Creative);
                }
                else
                {
                    switch (minecraft->GetMiniGame()->GetId())
                    {
                    case eMiniGameId_Lobby:
                        SetMusicID(eMusicType_Overworld);
                        break;
                    case eMiniGameId_Battle:
                        SetMusicID(eMusicType_Battle);
                        m_musicDelay = MINIGAME_MUSIC_DELAY;
                        break;
                    case eMiniGameId_Tumble:
                        SetMusicID(eMusicType_Tumble);
                        m_musicDelay = MINIGAME_MUSIC_DELAY;
                        break;
                    case eMiniGameId_Glide:
                        SetMusicID(eMusicType_Glide);
                        m_musicDelay = MINIGAME_MUSIC_DELAY;
                        break;
                    case eMiniGameId_Build:
                        SetMusicID(eMusicType_Build);
                        m_musicDelay = MINIGAME_MUSIC_DELAY;
                        break;
                    default:
                        SetMusicID(eMusicType_Overworld);
                        break;
                    }
                }
            }
            SetIsPlayingNetherMusic(false);
            SetIsPlayingEndMusic(false);
        }

        if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getId()
            == SHORT_MUSIC_DELAY_TEXTURE_PACK_ID)
            m_musicDelay = m_random->nextInt(200) + 100;

        m_streamState = eMusicStreamState_Idle;
        break;
    }

    default:
        break;
    }

    if (m_platformSoundEngine->isStreamComplete())
    {
#ifdef MINECRAFT_PC
        std::printf("[pc] music: %s ended\n", m_streamName);
        std::fflush(stdout);
#endif
        SetIsPlayingStreamingCDMusic(false);
        SetIsPlayingStreamingGameMusic(false);
        m_streamState = eMusicStreamState_Completed;
    }
}

void SoundEngine::playMusicTick()
{
    playMusicUpdate();
}

void SoundEngine::init(Options*)
{
    m_platformSoundEngine->init(NULL, NULL);
    m_openStreamThread = NULL;
}

bool SoundEngine::RouteToDRC(void*, void* soundData)
{
    return static_cast<AudioInfo*>(soundData)->routeToDRC;
}

void SoundEngine::updateMiles()
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float forwardX = 0.0f;
    float forwardY = 0.0f;
    float forwardZ = 1.0f;

    if (m_validListenerCount == 1)
    {
        for (int i = 0; i < MAX_LISTENERS; ++i)
        {
            if (m_listeners[i].valid)
            {
                x = m_listeners[i].x;
                y = m_listeners[i].y;
                z = -m_listeners[i].z;
                forwardX = -m_listeners[i].forwardX;
                forwardY = m_listeners[i].forwardY;
                forwardZ = m_listeners[i].forwardZ;
                break;
            }
        }
    }

    bool systemMusicPlaying;
    m_platformSoundEngine->tick(this, x, y, z, forwardX, forwardY, forwardZ, ConfigureSound, UpdateSound, RouteToDRC,
                                &systemMusicPlaying);
    updateSystemMusicPlaying(systemMusicPlaying);
}

void SoundEngine::tick(boost::shared_ptr<Mob>* mobs, float partialTicks)
{
    ConsoleSoundEngine::tick();

    int validListeners = 0;
    if (mobs == NULL)
    {
        m_listeners[0].z = 0.0f;
        validListeners = 1;
        m_listeners[0].y = 0.0f;
        m_listeners[0].forwardX = 0.0f;
        m_listeners[0].forwardY = 0.0f;
        m_listeners[0].x = 0.0f;
        m_listeners[0].forwardZ = 1.0f;
    }
    else
    {
        for (int i = 0; i < MAX_LISTENERS; ++i)
        {
            if (mobs[i] != NULL)
            {
                m_listeners[i].valid = true;
                float x = static_cast<float>(mobs[i]->mXo + (mobs[i]->mX - mobs[i]->mXo) * partialTicks);
                float y = static_cast<float>(mobs[i]->mYo + (mobs[i]->mY - mobs[i]->mYo) * partialTicks);
                float z = static_cast<float>(mobs[i]->mZo + (mobs[i]->mZ - mobs[i]->mZo) * partialTicks);
                float yRot = mobs[i]->mYRotO + (mobs[i]->mYRot - mobs[i]->mYRotO) * partialTicks;
                float angle = -(yRot * Mth::DEG_TO_RAD) - PI;
                float forwardZ = cosf(angle);
                float forwardX = sinf(angle);
                m_listeners[i].x = x;
                m_listeners[i].y = y;
                m_listeners[i].z = z;
                m_listeners[i].forwardX = forwardX;
                m_listeners[i].forwardY = 0.0f;
                m_listeners[i].forwardZ = forwardZ;
                ++validListeners;
            }
            else
            {
                m_listeners[i].valid = false;
            }
        }

        if (validListeners == 0)
        {
            m_listeners[0].forwardX = 0.0f;
            m_listeners[0].forwardY = 0.0f;
            validListeners = 1;
            m_listeners[0].z = 0.0f;
            m_listeners[0].forwardZ = 1.0f;
            m_listeners[0].x = 0.0f;
            m_listeners[0].y = 0.0f;
        }
    }

    m_validListenerCount = validListeners;
    updateMiles();
}

void SoundEngine::add(const std::wstring&, File*)
{
}

void SoundEngine::addMusic(const std::wstring&, File*)
{
}

void SoundEngine::addStreaming(const std::wstring&, File*)
{
}

char* SoundEngine::ConvertSoundPathToName(const std::wstring& path, bool replaceSpaces)
{
    static char name[260];
    const std::size_t length = path.size();
    for (std::size_t i = 0; i < length; ++i)
    {
        wchar_t value = path[i];
        if (value == L'.')
            value = L'/';
        else if (replaceSpaces && value == L' ')
            value = L'_';
        name[i] = static_cast<char>(value);
    }
    name[length] = 0;
    return name;
}

void SoundEngine::FlagMusicEventStart()
{
#ifdef MINECRAFT_PC
    std::printf("[pc] music: the round started\n");
    std::fflush(stdout);
#endif
    m_waitingForMusicStartEvent = false;
}

void SoundEngine::updateSoundEffectVolume(float volume)
{
    setMasterSoundEffectVolume(volume);
}
