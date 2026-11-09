#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_EQUALFIRST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_EQUALFIRST_HPP

#include <functional>
#include <string.h>
#include <utility>

namespace nn
{
namespace nex
{

struct EqualFirst : public std::binary_function<const char*, std::pair<char*, char*>, bool>
{
    bool operator()(const char* pKey, const std::pair<char*, char*>& rEntry) const
    {
        return strcmp(pKey, rEntry.first) == 0;
    }
};

}
}

#endif
