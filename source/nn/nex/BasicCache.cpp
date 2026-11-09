#include "nn/nex/BasicCache.hpp"

#include "nn/nex/CacheManager.hpp"

namespace nn
{
namespace nex
{

BasicCache::~BasicCache()
{
    CacheManager::GetInstance()->UnregisterCache(m_name);
}

}
}
