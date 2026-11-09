#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BASICCACHE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BASICCACHE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class BasicCache : public RootObject
{
public:
    virtual ~BasicCache();

private:
    unsigned int m_field0;
    String m_name;
};

}
}

#endif
