#include "platform/pc/PcKnownIdentities.hpp"

#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

#include "game/core/CriticalSection.hpp"

#include <map>
#include <stdio.h>
#include <string.h>
#include <string>

namespace
{
const char FILE_NAME[] = "known-identities.txt";

struct KeyLock
{
    KeyLock() { InitializeCriticalSection(&lock); }
    OSFastMutex lock;
};

OSFastMutex& keyLock()
{
    static KeyLock shared;
    return shared.lock;
}

bool loaded = false;
std::map<std::string, std::string> keys;

std::string toHex(const unsigned char* data, int len)
{
    static const char digits[] = "0123456789abcdef";
    std::string out;
    for (int i = 0; i < len; i++)
    {
        out += digits[data[i] >> 4];
        out += digits[data[i] & 15];
    }
    return out;
}

std::string filePath()
{
    return PcSaveRoot() + "/" + FILE_NAME;
}

void load()
{
    loaded = true;
    FILE* f = fopen(filePath().c_str(), "r");
    if (f == 0)
        return;
    char uuid[64], key[128];
    while (fscanf(f, "%63s %127s", uuid, key) == 2)
    {
        if (strlen(uuid) == 32 && strlen(key) == 64)
            keys[uuid] = key;
    }
    fclose(f);
}

void save()
{
    std::string path = filePath();
    std::string tmp = path + ".tmp";
    FILE* f = fopen(tmp.c_str(), "w");
    if (f == 0)
        return;
    for (std::map<std::string, std::string>::iterator it = keys.begin(); it != keys.end(); ++it)
        fprintf(f, "%s %s\n", it->first.c_str(), it->second.c_str());
    if (fclose(f) == 0)
        PcOsReplaceFile(tmp.c_str(), path.c_str());
}
}

bool PcKnownIdentityCheck(const unsigned char uuid[16], const unsigned char key[32])
{
    std::string id = toHex(uuid, 16);
    std::string hex = toHex(key, 32);
    EnterCriticalSection(&keyLock());
    if (!loaded)
        load();
    bool ok = true;
    std::map<std::string, std::string>::iterator it = keys.find(id);
    if (it == keys.end())
    {
        keys[id] = hex;
        save();
    }
    else
        ok = it->second == hex;
    LeaveCriticalSection(&keyLock());
    return ok;
}
