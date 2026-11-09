#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPDATA_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class HTTPData : public RootObject
{
public:
    virtual ~HTTPData() {}

    virtual unsigned int GetContentLength() const = 0;
    virtual unsigned int Read(void* pBuffer, unsigned int uiSize) = 0;
};

}
}

#endif
