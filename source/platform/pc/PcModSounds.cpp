#include "platform/pc/PcModSounds.hpp"

#include "platform/pc/PcNativeMod.h"
#include "platform/pc/PcNativeMods.hpp"

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>
#include <string>

#include "dr_mp3.h"

extern "C" int stb_vorbis_decode_memory(const unsigned char* mem, int len, int* channels, int* sampleRate,
                                        short** output);

namespace
{
const int MAX_VARIANTS = 16;
const long MAX_FILE = 16 * 1024 * 1024;
const long long MAX_PCM = 32 * 1024 * 1024;

std::map<const SoundEvent*, std::vector<std::vector<char> > > sounds;

std::wstring Widen(const std::string& s)
{
    std::wstring w;
    for (size_t i = 0; i < s.size(); i++)
        w += (wchar_t)(unsigned char)s[i];
    return w;
}

bool GoodName(const char* name)
{
    if (name == 0 || name[0] == 0 || strlen(name) > 64)
        return false;
    for (const char* p = name; *p != 0; p++)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_' || *p == '.'))
            return false;
    return true;
}

bool ReadFile(const std::string& path, std::vector<char>* out, std::string* why)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
    {
        *why = "no such file";
        return false;
    }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (len < 16 || len > MAX_FILE)
    {
        fclose(fp);
        *why = "too small or larger than 16 MB";
        return false;
    }
    out->resize(len);
    size_t got = fread(&(*out)[0], 1, len, fp);
    fclose(fp);
    if ((long)got != len)
    {
        *why = "could not be read";
        return false;
    }
    return true;
}

void Put32(char* p, unsigned int v)
{
    p[0] = (char)v;
    p[1] = (char)(v >> 8);
    p[2] = (char)(v >> 16);
    p[3] = (char)(v >> 24);
}

void Put16(char* p, unsigned int v)
{
    p[0] = (char)v;
    p[1] = (char)(v >> 8);
}

void MakeWav(const short* pcm, long long frames, int channels, int rate, std::vector<char>* out)
{
    unsigned int bytes = (unsigned int)(frames * channels * 2);
    out->assign(44 + bytes, 0);
    char* h = &(*out)[0];
    memcpy(h, "RIFF", 4);
    Put32(h + 4, 36 + bytes);
    memcpy(h + 8, "WAVEfmt ", 8);
    Put32(h + 16, 16);
    Put16(h + 20, 1);
    Put16(h + 22, channels);
    Put32(h + 24, rate);
    Put32(h + 28, rate * channels * 2);
    Put16(h + 32, channels * 2);
    Put16(h + 34, 16);
    memcpy(h + 36, "data", 4);
    Put32(h + 40, bytes);
    memcpy(h + 44, pcm, bytes);
}

bool EndsWith(const std::string& s, const char* end)
{
    size_t n = strlen(end);
    if (s.size() < n)
        return false;
    for (size_t i = 0; i < n; i++)
        if (tolower((unsigned char)s[s.size() - n + i]) != end[i])
            return false;
    return true;
}

bool LoadSound(const std::string& path, std::vector<char>* out, std::string* why)
{
    std::vector<char> file;
    if (!ReadFile(path, &file, why))
        return false;
    if (EndsWith(path, ".wav"))
    {
        if (file.size() < 44 || memcmp(&file[0], "RIFF", 4) != 0 || memcmp(&file[8], "WAVE", 4) != 0)
        {
            *why = "not a WAV file (RIFF/WAVE)";
            return false;
        }
        out->swap(file);
        return true;
    }
    if (EndsWith(path, ".ogg"))
    {
        int channels = 0, rate = 0;
        short* pcm = 0;
        int frames = stb_vorbis_decode_memory((const unsigned char*)&file[0], (int)file.size(), &channels, &rate, &pcm);
        if (frames <= 0 || pcm == 0 || channels < 1 || channels > 2)
        {
            free(pcm);
            *why = "not an Ogg Vorbis file this game can play (mono or stereo)";
            return false;
        }
        if ((long long)frames * channels * 2 > MAX_PCM)
        {
            free(pcm);
            *why = "longer than about 3 minutes once decoded";
            return false;
        }
        MakeWav(pcm, frames, channels, rate, out);
        free(pcm);
        return true;
    }
    if (EndsWith(path, ".mp3"))
    {
        drmp3_config cfg;
        drmp3_uint64 frames = 0;
        drmp3_int16* pcm = drmp3_open_memory_and_read_pcm_frames_s16(&file[0], file.size(), &cfg, &frames, 0);
        if (pcm == 0 || frames == 0 || cfg.channels < 1 || cfg.channels > 2)
        {
            drmp3_free(pcm, 0);
            *why = "not an MP3 file this game can play (mono or stereo)";
            return false;
        }
        if ((long long)frames * cfg.channels * 2 > MAX_PCM)
        {
            drmp3_free(pcm, 0);
            *why = "longer than about 3 minutes once decoded";
            return false;
        }
        MakeWav(pcm, (long long)frames, (int)cfg.channels, (int)cfg.sampleRate, out);
        drmp3_free(pcm, 0);
        return true;
    }
    *why = "not a .wav, .ogg or .mp3 file";
    return false;
}

void* AddSound(const char* name, const char* files, float range)
{
    const char* id = PcNativeModsIdFor(__builtin_return_address(0));
    if (id == 0 || !GoodName(name) || files == 0 || files[0] == 0)
    {
        printf("[pc] native %s: sound \"%s\" not added\n", id != 0 ? id : "?", name != 0 ? name : "");
        fflush(stdout);
        return 0;
    }
    std::string dir = PcNativeModsDirectoryOf(id);
    std::vector<std::vector<char> > variants;
    std::string list = files;
    size_t at = 0;
    while (at <= list.size() && variants.size() < (size_t)MAX_VARIANTS)
    {
        size_t semi = list.find(';', at);
        if (semi == std::string::npos)
            semi = list.size();
        std::string file = list.substr(at, semi - at);
        at = semi + 1;
        if (file.empty())
            continue;
        std::vector<char> data;
        std::string why;
        if (file.find("..") != std::string::npos || !LoadSound(dir + "/sounds/" + file, &data, &why))
        {
            if (why.empty())
                why = "a path outside the mod";
            printf("[pc] native %s: sound file sounds/%s: %s\n", id, file.c_str(), why.c_str());
            fflush(stdout);
            continue;
        }
        variants.push_back(data);
    }
    if (variants.empty())
    {
        printf("[pc] native %s: sound \"%s\" has no file it can play; not added\n", id, name);
        fflush(stdout);
        return 0;
    }
    SoundEvent* ev = new SoundEvent(Widen(std::string(id) + ":" + name), SoundEvent::eType_Sound,
                                    range > 0.0f ? range : 16.0f);
    SoundEvents::PcRegisterSound(ev);
    sounds[ev] = variants;
    return ev;
}
}

const std::vector<char>* PcModSoundFile(const SoundEvent* sound)
{
    if (sounds.empty() || sound == 0)
        return 0;
    std::map<const SoundEvent*, std::vector<std::vector<char> > >::iterator it = sounds.find(sound);
    if (it == sounds.end())
        return 0;
    return &it->second[rand() % it->second.size()];
}

void PcModSoundsFillRegister(PcNativeRegister* reg)
{
    reg->addSound = AddSound;
}
