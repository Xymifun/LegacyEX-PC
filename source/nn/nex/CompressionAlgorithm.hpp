#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_COMPRESSIONALGORITHM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_COMPRESSIONALGORITHM_HPP

#include "nn/nex/CompressionPlugin.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/PluginObject.hpp"
#include "nn/nex/ProfilingUnit.hpp"

namespace nn
{
namespace nex
{

class Buffer;

class CompressionAlgorithm : public PluginObject
{
public:
    CompressionAlgorithm();
    virtual ~CompressionAlgorithm();

    virtual int CompressImpl(const Buffer& rSource, Buffer* pDestination) = 0;
    virtual int DecompressImpl(const Buffer& rSource, Buffer* pDestination) = 0;

    int Compress(const Buffer& rSource, Buffer* pDestination);
    int Compress(Buffer* pBuffer);
    int Decompress(const Buffer& rSource, Buffer* pDestination);
    int Decompress(Buffer* pBuffer);

protected:
    void RecordCompressionStats(const Buffer& rSource, const Buffer& rDestination);

private:
    unsigned int m_uiTotalSourceLength;
    unsigned int m_uiTotalDestinationLength;
    CriticalSection m_criticalSection;
    ProfilingUnit m_compressionProfile;
    ProfilingUnit m_decompressionProfile;
};

}
}

#endif
