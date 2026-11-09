#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPWRITER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPWRITER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class HTTPWriter : public RootObject
{
public:
    virtual ~HTTPWriter() {}

    virtual bool Write(const void* pBuffer, unsigned int uiSize) = 0;
};

}
}

#endif
