#include "nn/nex/CacheManager.hpp"

#include "nn/nex/GlobalVariables.hpp"

namespace nn
{
namespace nex
{

CacheManager::CacheManager()
{
}

CacheManager::~CacheManager()
{
}

CacheManager* CacheManager::GetInstance()
{
    return &GlobalVariables::GetInstancePtr()->GetCacheManager();
}

bool CacheManager::UnregisterCache(const String& name)
{
    bool erased = false;
    CacheMap::iterator found = m_caches.find(name);
    if (found != m_caches.end())
    {
        m_caches.erase(found);
        erased = true;
    }
    return erased;
}

}
}
