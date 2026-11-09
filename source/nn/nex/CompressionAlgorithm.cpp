#include "nn/nex/CompressionAlgorithm.hpp"

#include "nn/nex/Buffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int DECOMPRESS_SCRATCH_SIZE = 2000;

}

CompressionAlgorithm::CompressionAlgorithm()
    : m_compressionProfile(L"Compression", 0, L""),
      m_decompressionProfile(L"Decompression", 0, L"")
{
    m_uiTotalSourceLength = 0;
    m_uiTotalDestinationLength = 0;
}

CompressionAlgorithm::~CompressionAlgorithm()
{
}

void CompressionAlgorithm::RecordCompressionStats(const Buffer& rSource, const Buffer& rDestination)
{
    m_uiTotalSourceLength += rSource.GetLength();
    m_uiTotalDestinationLength += rDestination.GetLength();
}

int CompressionAlgorithm::Compress(const Buffer& rSource, Buffer* pDestination)
{
    ScopedCS lock(&m_criticalSection);
    int iStatus = CompressImpl(rSource, pDestination);
    RecordCompressionStats(rSource, *pDestination);
    return iStatus;
}

int CompressionAlgorithm::Compress(Buffer* pBuffer)
{
    ScopedCS lock(&m_criticalSection);
    Buffer compressed(pBuffer->GetRealSize() - pBuffer->GetHeadShiftSize(), 0);
    int iStatus = CompressImpl(*pBuffer, &compressed);
    RecordCompressionStats(*pBuffer, compressed);
    *pBuffer = compressed;
    return iStatus;
}

int CompressionAlgorithm::Decompress(const Buffer& rSource, Buffer* pDestination)
{
    ScopedCS lock(&m_criticalSection);
    return DecompressImpl(rSource, pDestination);
}

int CompressionAlgorithm::Decompress(Buffer* pBuffer)
{
    ScopedCS lock(&m_criticalSection);
    Buffer decompressed(DECOMPRESS_SCRATCH_SIZE, 0);
    int iStatus = DecompressImpl(*pBuffer, &decompressed);
    *pBuffer = decompressed;
    return iStatus;
}

}
}
