#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CACHEMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CACHEMANAGER_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class BasicCache;

class CacheManager : public RootObject
{
public:
    CacheManager();
    ~CacheManager();

    static CacheManager* GetInstance();

    bool UnregisterCache(const String& name);

private:
    typedef std::map<String, BasicCache*, std::less<String>, MemAllocator<std::pair<const String, BasicCache*> > >
        CacheMap;

    unsigned int m_field0;
    CacheMap m_caches;
};

}
}

#endif
