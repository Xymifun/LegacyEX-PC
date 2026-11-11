#include "platform/pc/PcModStorage.hpp"

#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcKeyBindings.hpp"
#include "platform/pc/PcNativeMod.h"
#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/Tag.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/generation/Player.hpp"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <deque>

namespace
{
const int MAX_DATA = 4 * 1024 * 1024;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct ConfigLine
{
    std::string key;
    std::string value;
    std::string comment;
};

struct ModConfig
{
    bool loaded;
    std::deque<ConfigLine> lines;
};
std::map<std::string, ModConfig> configs;

std::wstring Widen(const std::string& s)
{
    std::wstring w;
    for (size_t i = 0; i < s.size(); i++)
        w += (wchar_t)(unsigned char)s[i];
    return w;
}

std::string Narrow(const std::wstring& w)
{
    std::string s;
    for (size_t i = 0; i < w.size(); i++)
        s += w[i] < 0x80 ? (char)w[i] : '?';
    return s;
}

std::wstring FromUtf8(const char* s)
{
    std::wstring w;
    const unsigned char* p = (const unsigned char*)s;
    while (p != 0 && *p != 0)
    {
        unsigned int c = *p++;
        int more = 0;
        if (c >= 0xF0)
        {
            c &= 0x07;
            more = 3;
        }
        else if (c >= 0xE0)
        {
            c &= 0x0F;
            more = 2;
        }
        else if (c >= 0xC0)
        {
            c &= 0x1F;
            more = 1;
        }
        for (; more > 0 && (*p & 0xC0) == 0x80; more--)
            c = (c << 6) | (*p++ & 0x3F);
        w += (wchar_t)c;
    }
    return w;
}

std::string Trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t'))
        a++;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n'))
        b--;
    return s.substr(a, b - a);
}

std::string ConfigPath(const std::string& id)
{
    return PcSaveRoot() + "/config/" + id + ".cfg";
}

void LoadConfig(const std::string& id, ModConfig* c)
{
    c->loaded = true;
    FILE* fp = fopen(ConfigPath(id).c_str(), "r");
    if (fp == 0)
        return;
    char buf[2048];
    std::string comment;
    while (fgets(buf, sizeof(buf), fp) != 0)
    {
        std::string s = Trim(buf);
        if (s.empty())
            continue;
        if (s[0] == '#')
        {
            std::string text = Trim(s.substr(1));
            if (text.compare(0, 20, "settings of the mod ") == 0)
                continue;
            comment += (comment.empty() ? "" : "\n") + text;
            continue;
        }
        size_t eq = s.find('=');
        if (eq == std::string::npos)
            continue;
        ConfigLine l;
        l.key = Trim(s.substr(0, eq));
        l.value = Trim(s.substr(eq + 1));
        l.comment = comment;
        comment.clear();
        if (!l.key.empty())
            c->lines.push_back(l);
    }
    fclose(fp);
}

void SaveConfig(const std::string& id, const ModConfig& c)
{
    PcOsMakeDirectory((PcSaveRoot() + "/config").c_str());
    FILE* fp = fopen(ConfigPath(id).c_str(), "w");
    if (fp == 0)
        return;
    fprintf(fp, "# settings of the mod %s (key=value; edit while the game is closed)\n\n", id.c_str());
    for (size_t i = 0; i < c.lines.size(); i++)
    {
        const std::string& com = c.lines[i].comment;
        size_t at = 0;
        while (!com.empty() && at <= com.size())
        {
            size_t nl = com.find('\n', at);
            if (nl == std::string::npos)
                nl = com.size();
            fprintf(fp, "# %s\n", com.substr(at, nl - at).c_str());
            at = nl + 1;
        }
        fprintf(fp, "%s=%s\n\n", c.lines[i].key.c_str(), c.lines[i].value.c_str());
    }
    fclose(fp);
}

bool GoodKey(const char* key)
{
    if (key == 0 || key[0] == 0 || strlen(key) > 64)
        return false;
    for (const char* p = key; *p != 0; p++)
        if (*p == '=' || *p == '#' || *p == '\n' || *p == '\r')
            return false;
    return true;
}

std::string OneLine(const char* s)
{
    std::string v = s != 0 ? s : "";
    for (size_t i = 0; i < v.size(); i++)
        if (v[i] == '\n' || v[i] == '\r')
            v[i] = ' ';
    return Trim(v);
}

ModConfig* ConfigFor(const std::string& id)
{
    ModConfig& c = configs[id];
    if (!c.loaded)
        LoadConfig(id, &c);
    return &c;
}

const char* ConfigValue(const char* id, const char* key, const char* def, const char* comment)
{
    if (id == 0 || !GoodKey(key))
        return def;
    pthread_mutex_lock(&lock);
    ModConfig* c = ConfigFor(id);
    for (size_t i = 0; i < c->lines.size(); i++)
    {
        if (c->lines[i].key == key)
        {
            const char* v = c->lines[i].value.c_str();
            pthread_mutex_unlock(&lock);
            return v;
        }
    }
    ConfigLine l;
    l.key = key;
    l.value = OneLine(def);
    l.comment = comment != 0 ? comment : "";
    c->lines.push_back(l);
    SaveConfig(id, *c);
    const char* v = c->lines.back().value.c_str();
    pthread_mutex_unlock(&lock);
    return v;
}

const char* GetConfig(const char* key, const char* def, const char* comment)
{
    return ConfigValue(PcNativeModsIdFor(__builtin_return_address(0)), key, def, comment);
}

int GetConfigInt(const char* key, int def, const char* comment)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", def);
    const char* v = ConfigValue(PcNativeModsIdFor(__builtin_return_address(0)), key, buf, comment);
    char* end = 0;
    long n = strtol(v, &end, 10);
    if (end == v || *end != 0)
        return def;
    return (int)n;
}

void SetConfig(const char* key, const char* value)
{
    const char* id = PcNativeModsIdFor(__builtin_return_address(0));
    if (id == 0 || !GoodKey(key))
        return;
    pthread_mutex_lock(&lock);
    ModConfig* c = ConfigFor(id);
    bool found = false;
    for (size_t i = 0; i < c->lines.size(); i++)
    {
        if (c->lines[i].key == key)
        {
            c->lines[i].value = OneLine(value);
            found = true;
        }
    }
    if (!found)
    {
        ConfigLine l;
        l.key = key;
        l.value = OneLine(value);
        c->lines.push_back(l);
    }
    SaveConfig(id, *c);
    pthread_mutex_unlock(&lock);
}

LevelData* HostWorld()
{
    if (gMinecraftServer == 0)
        return 0;
    ServerLevel* level = gMinecraftServer->getLevel(0);
    return level != 0 ? level->getLevelData() : 0;
}

const void* GetData(PcModData* data, const char* id, int* size)
{
    *size = 0;
    if (data == 0 || id == 0)
        return 0;
    pthread_mutex_lock(&lock);
    const void* p = 0;
    PcModData::iterator it = data->find(id);
    if (it != data->end() && !it->second.empty())
    {
        p = &it->second[0];
        *size = (int)it->second.size();
    }
    pthread_mutex_unlock(&lock);
    return p;
}

void SetData(PcModData* data, const char* id, const void* bytes, int size)
{
    if (data == 0 || id == 0)
        return;
    if (size < 0 || size > MAX_DATA || (size > 0 && bytes == 0))
    {
        printf("[pc] native %s: data of %d bytes not kept (up to %d)\n", id, size, MAX_DATA);
        fflush(stdout);
        return;
    }
    pthread_mutex_lock(&lock);
    if (size == 0)
        data->erase(id);
    else
        (*data)[id].assign((const unsigned char*)bytes, (const unsigned char*)bytes + size);
    pthread_mutex_unlock(&lock);
}

const void* GetWorldData(int* size)
{
    LevelData* ld = HostWorld();
    return GetData(ld != 0 ? &ld->mModData : 0, PcNativeModsIdFor(__builtin_return_address(0)), size);
}

void SetWorldData(const void* data, int size)
{
    LevelData* ld = HostWorld();
    SetData(ld != 0 ? &ld->mModData : 0, PcNativeModsIdFor(__builtin_return_address(0)), data, size);
}

const void* GetPlayerData(void* player, int* size)
{
    return GetData(player != 0 ? &((Player*)player)->mModData : 0, PcNativeModsIdFor(__builtin_return_address(0)),
                   size);
}

void SetPlayerData(void* player, const void* data, int size)
{
    SetData(player != 0 ? &((Player*)player)->mModData : 0, PcNativeModsIdFor(__builtin_return_address(0)), data,
            size);
}

int AddKey(const char* name, const char* label, int key)
{
    const char* id = PcNativeModsIdFor(__builtin_return_address(0));
    bool ok = id != 0 && name != 0 && name[0] != 0 && strlen(name) <= 64;
    for (const char* p = name; ok && *p != 0; p++)
        ok = (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_';
    int act = ok ? PcKeyBindingsAddMod(std::string(id) + ":" + name, FromUtf8(label != 0 ? label : name), key) : -1;
    if (act < 0)
    {
        printf("[pc] native %s: key \"%s\" not added\n", id != 0 ? id : "?", name != 0 ? name : "");
        fflush(stdout);
        return 0;
    }
    return act;
}

int KeyDown(int key)
{
    return PcInputModKeyHeld(key) ? 1 : 0;
}

int KeyPressed(int key)
{
    return PcInputModKeyPresses(key);
}
}

void PcModStorageRead(CompoundTag* tag, PcModData* out)
{
    pthread_mutex_lock(&lock);
    out->clear();
    if (tag->contains(L"PcModData", Tag::TAG_COMPOUND))
    {
        CompoundTag* c = tag->getCompound(L"PcModData");
        boost::unordered_map<std::wstring, Tag*>* all = c->getAllTagsMap();
        for (boost::unordered_map<std::wstring, Tag*>::iterator it = all->begin(); it != all->end(); ++it)
        {
            if (it->second->getId() != Tag::TAG_BYTE_ARRAY)
                continue;
            arrayWithLength<unsigned char> a = c->getByteArray(it->first);
            if (a.length > 0)
                (*out)[Narrow(it->first)].assign(a.data, a.data + a.length);
        }
    }
    pthread_mutex_unlock(&lock);
}

void PcModStorageWrite(CompoundTag* tag, const PcModData& data)
{
    pthread_mutex_lock(&lock);
    if (!data.empty())
    {
        CompoundTag* c = new CompoundTag();
        for (PcModData::const_iterator it = data.begin(); it != data.end(); ++it)
        {
            arrayWithLength<unsigned char> a((unsigned int)it->second.size(), true);
            memcpy(a.data, &it->second[0], it->second.size());
            c->putByteArray(Widen(it->first), a);
        }
        tag->put(L"PcModData", c);
    }
    pthread_mutex_unlock(&lock);
}

void PcModStorageFillRegister(PcNativeRegister* reg)
{
    reg->getConfig = GetConfig;
    reg->getConfigInt = GetConfigInt;
    reg->setConfig = SetConfig;
    reg->addKey = AddKey;
}

void PcModStorageFillHost(PcNativeHost* host)
{
    host->getConfig = GetConfig;
    host->getConfigInt = GetConfigInt;
    host->setConfig = SetConfig;
    host->getWorldData = GetWorldData;
    host->setWorldData = SetWorldData;
    host->getPlayerData = GetPlayerData;
    host->setPlayerData = SetPlayerData;
    host->keyDown = KeyDown;
    host->keyPressed = KeyPressed;
}
