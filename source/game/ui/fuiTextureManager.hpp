#ifndef GAME_UI_FUITEXTUREMANAGER_HPP
#define GAME_UI_FUITEXTUREMANAGER_HPP

#include <stdint.h>

#include "game/core/CriticalSection.hpp"

struct fuiBitmap;
struct fuiFile;

class fuiTextureManager
{
private:
    struct TextureEntry
    {
        fuiBitmap* bitmap;
        fuiFile* file;
        uint64_t lastUsed;
    };

    unsigned int textureCount;
    TextureEntry* textures;
    int managedTextureCount;
    int managedTextureMemory;
    unsigned int peakTextureCount;
    unsigned int peakTextureMemory;
    int tickCount;
    bool initialised;
    OSFastMutex mutex;

public:
    fuiTextureManager();
    void initialise();
    void removeManagedTexture(fuiFile* file, fuiBitmap* bitmap);
    void manageTexture(fuiFile* file, fuiBitmap* bitmap);
    void tick();
};

#endif
