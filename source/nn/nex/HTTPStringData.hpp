#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPSTRINGDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPSTRINGDATA_HPP

#include "nn/nex/HTTPData.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class HTTPStringData : public HTTPData
{
public:
    explicit HTTPStringData(const String& rData);
    HTTPStringData(const HTTPStringData& rOther);
    virtual ~HTTPStringData();

    HTTPStringData& operator=(const HTTPStringData& rOther);

    virtual unsigned int GetContentLength() const;
    virtual unsigned int Read(void* pBuffer, unsigned int uiSize);

private:
    String m_data;
    unsigned int m_position;
};

}
}

#endif
