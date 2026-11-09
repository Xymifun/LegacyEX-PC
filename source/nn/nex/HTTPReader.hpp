#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPREADER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPREADER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class HTTPReader : public RootObject
{
public:
    virtual ~HTTPReader() {}

    virtual unsigned int Read(void* pBuffer, unsigned int uiSize) = 0;
    virtual unsigned int GetContentLength() const = 0;
};

}
}

#endif
