#include "game/core/MemoryTracker.hpp"

#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/ByteBuffer.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IntBuffer.hpp"

static boost::unordered_map<int, int> displayLists;
static std::vector<int> textureObjects;

int MemoryTracker::genLists(int count)
{
    int list = GlStateManager::genLists(count);
    displayLists.insert(std::make_pair(list, count));
    return list;
}

void MemoryTracker::releaseLists(int list)
{
    boost::unordered_map<int, int>::iterator entry = displayLists.find(list);
    if (entry != displayLists.end())
    {
        GlStateManager::deleteLists(list, entry->second);
        displayLists.erase(entry);
    }
}

int MemoryTracker::genTextures()
{
    int texture = GlStateManager::genTexture();
    textureObjects.push_back(texture);
    return texture;
}

void MemoryTracker::releaseTextures()
{
    for (unsigned int i = 0; i < textureObjects.size(); ++i)
    {
        GlStateManager::deleteTexture(textureObjects.at(i));
    }

    textureObjects.clear();
}

void MemoryTracker::release()
{
    for (boost::unordered_map<int, int>::iterator entry = displayLists.begin(); entry != displayLists.end(); ++entry)
    {
        GlStateManager::deleteLists(entry->first, entry->second);
    }

    displayLists.clear();
    releaseTextures();
}

ByteBuffer* MemoryTracker::createByteBuffer(int capacity)
{
    return ByteBuffer::allocate(capacity);
}

IntBuffer* MemoryTracker::createIntBuffer(int capacity)
{
    return createByteBuffer(capacity << 2)->asIntBuffer();
}

FloatBuffer* MemoryTracker::createFloatBuffer(int capacity)
{
    return createByteBuffer(capacity << 2)->asFloatBuffer();
}
