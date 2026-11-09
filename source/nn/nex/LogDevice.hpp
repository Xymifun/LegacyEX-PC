#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICE_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class LogDevice : public RootObject
{
public:
    virtual ~LogDevice() {}
    virtual void Output(const wchar_t* pText) = 0;
};

}
}

#endif
