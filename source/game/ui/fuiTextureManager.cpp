#include "game/ui/fuiTextureManager.hpp"

#include "game/client/render/Renderer.hpp"
#include "game/core/System.hpp"
#include "game/ui/fuiBitmap.hpp"

fuiTextureManager::fuiTextureManager()
{
    tickCount = 0;
    textures = 0;
    peakTextureCount = 0;
    peakTextureMemory = 0;
    managedTextureMemory = 0;
    textureCount = 0;
    managedTextureCount = 0;
    initialised = false;
    InitializeCriticalSection(&mutex);
}

void fuiTextureManager::initialise()
{
    textureCount = (unsigned int)gRenderer->GetMaxTextures();
    TextureEntry* entries = static_cast<TextureEntry*>(operator new[](textureCount * sizeof(TextureEntry)));
    textures = entries;
    int count = (int)textureCount;
    for (int i = 0; i < count; i++)
    {
        entries[i].bitmap = 0;
        entries[i].file = 0;
        entries[i].lastUsed = 0;
    }
    initialised = true;
}

void fuiTextureManager::removeManagedTexture(fuiFile*, fuiBitmap* bitmap)
{
    if (!initialised)
        initialise();
    if (bitmap->textureId != -1)
    {
        TextureEntry& entry = textures[bitmap->textureId];
        if (entry.bitmap != 0)
        {
            EnterCriticalSection(&mutex);
            --managedTextureCount;
            managedTextureMemory -= (int)(bitmap->height * bitmap->width * 4);
            entry.bitmap = 0;
            entry.file = 0;
            entry.lastUsed = 0;
            bitmap->textureId = 0;
            LeaveCriticalSection(&mutex);
        }
    }
}

void fuiTextureManager::manageTexture(fuiFile* file, fuiBitmap* bitmap)
{
    if (!initialised)
        initialise();
    if (bitmap->textureId != -1)
    {
        TextureEntry& entry = textures[bitmap->textureId];
        if (entry.bitmap == 0)
        {
            EnterCriticalSection(&mutex);
            unsigned int count = (unsigned int)(managedTextureCount + 1);
            entry.file = file;
            entry.bitmap = bitmap;
            managedTextureCount = (int)count;
            if (peakTextureCount < count)
                peakTextureCount = count;
            unsigned int memory = (unsigned int)(managedTextureMemory + bitmap->height * bitmap->width * 4);
            managedTextureMemory = (int)memory;
            if (peakTextureMemory < memory)
                peakTextureMemory = memory;
            LeaveCriticalSection(&mutex);
        }
        uint64_t now = System::processTimeInMilliSecs();
        entry.lastUsed = now;
    }
}

void fuiTextureManager::tick()
{
    if (!initialised)
        initialise();
    ++tickCount;
    if ((tickCount % 20) == 0)
    {
        uint64_t now = System::processTimeInMilliSecs();
        EnterCriticalSection(&mutex);
        int count = (int)textureCount;
        int index = 0;
        while (index < count)
        {
            if (textures[index].lastUsed != 0)
            {
                int64_t age = (int64_t)(now - textures[index].lastUsed);
                int ageHigh = (int)(age >> 32);
                if (ageHigh >= 0 && (ageHigh > 0 || (unsigned int)age > 1000))
                {
                    gRenderer->TextureFree(textures[index].bitmap->textureId);
                    removeManagedTexture(textures[index].file, textures[index].bitmap);
                    count = (int)textureCount;
                }
            }
            ++index;
        }
        LeaveCriticalSection(&mutex);
    }
}
