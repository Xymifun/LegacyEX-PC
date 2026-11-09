#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPOSTOBJECTEVENTLISTENER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPOSTOBJECTEVENTLISTENER_HPP

namespace nn
{
namespace nex
{

class DataStorePostObjectEventListener
{
public:
    virtual ~DataStorePostObjectEventListener() {}

    virtual unsigned int PostChunkedBuffer(unsigned char* pBuffer, unsigned int uiSize, bool* pbLastChunk) = 0;
};

}
}

#endif
