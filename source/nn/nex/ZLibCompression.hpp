#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ZLIBCOMPRESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ZLIBCOMPRESSION_HPP

#include "nn/nex/CompressionAlgorithm.hpp"
#include "nn/nex/QuazalZlib.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class ZLibCompression : public CompressionAlgorithm
{
public:
    ZLibCompression();
    ZLibCompression(int iWindowBits, int iMemLevel);
    virtual ~ZLibCompression();

    virtual int CompressImpl(const Buffer& rSource, Buffer* pDestination);
    virtual int DecompressImpl(const Buffer& rSource, Buffer* pDestination);

private:
    struct Streams : public RootObject
    {
        z_stream m_inflate;
        z_stream m_deflate;
    };

    void Initialize(int iWindowBits, int iMemLevel);

    Streams* m_pStreams;
};

}
}

#endif
