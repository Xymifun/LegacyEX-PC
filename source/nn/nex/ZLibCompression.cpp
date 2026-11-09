#include "nn/nex/ZLibCompression.hpp"

#include <string.h>

#include "nn/nex/Buffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int USE_DEFAULT = -1;
const int DEFAULT_WINDOW_BITS = 15;
const int DEFAULT_MEM_LEVEL = 8;
const unsigned char STORED_MARKER = 0;
const unsigned char UNKNOWN_RATIO_MARKER = 0xFF;
const float MAX_RATIO_MARKER = 255.0f;
const unsigned int UNKNOWN_RATIO_EXPANSION = 0x41A;
const unsigned int MAX_INFLATE_ATTEMPTS = 2;
const int DECOMPRESS_STATUS_OK = 0;
const int DECOMPRESS_STATUS_FAILED = 1;
const int DECOMPRESS_STATUS_TRUNCATED = 2;

}

ZLibCompression::ZLibCompression()
{
    Initialize(DEFAULT_WINDOW_BITS, DEFAULT_MEM_LEVEL);
}

ZLibCompression::ZLibCompression(int iWindowBits, int iMemLevel)
{
    Initialize(iWindowBits == USE_DEFAULT ? DEFAULT_WINDOW_BITS : iWindowBits,
               iMemLevel == USE_DEFAULT ? DEFAULT_MEM_LEVEL : iMemLevel);
}

ZLibCompression::~ZLibCompression()
{
    quazal_inflateEnd(&m_pStreams->m_inflate);
    quazal_deflateEnd(&m_pStreams->m_deflate);
    delete m_pStreams;
    m_pStreams = 0;
}

void ZLibCompression::Initialize(int iWindowBits, int iMemLevel)
{
    m_pStreams = new Streams();
    m_pStreams->m_inflate.zalloc = QuazalCZlibAlloc;
    m_pStreams->m_inflate.zfree = QuazalCZlibFree;
    m_pStreams->m_inflate.opaque = 0;
    quazal_inflateInit_(&m_pStreams->m_inflate, ZLIB_VERSION, sizeof(z_stream));
    m_pStreams->m_deflate.zalloc = QuazalCZlibAlloc;
    m_pStreams->m_deflate.zfree = QuazalCZlibFree;
    m_pStreams->m_deflate.opaque = 0;
    quazal_deflateInit2_(&m_pStreams->m_deflate, Z_DEFAULT_COMPRESSION, Z_DEFLATED, iWindowBits, iMemLevel,
                         Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(z_stream));
}

int ZLibCompression::CompressImpl(const Buffer& rSource, Buffer* pDestination)
{
    quazal_deflateReset(&m_pStreams->m_deflate);
    if (pDestination->GetRealSize() - pDestination->GetHeadShiftSize() < rSource.GetLength() + 1)
    {
        pDestination->ResizeByRealSize(rSource.GetLength() + pDestination->GetHeadShiftSize() + 1);
    }

    z_stream* pStream = &m_pStreams->m_deflate;
    unsigned int uiAvailable = pDestination->GetRealSize() - pDestination->GetHeadShiftSize();
    pStream->avail_in = rSource.GetLength();
    pStream->next_in = rSource.GetData();
    pStream->next_out = pDestination->GetData() + 1;
    pStream->avail_out = uiAvailable - 1;
    if (quazal_deflate(pStream, Z_FINISH) == Z_STREAM_END && m_pStreams->m_deflate.total_out < rSource.GetLength())
    {
        unsigned int uiCompressedLength = m_pStreams->m_deflate.total_out;
        float fRatio = static_cast<float>(rSource.GetLength()) / static_cast<float>(uiCompressedLength) + 1.0f;
        unsigned char uiMarker = UNKNOWN_RATIO_MARKER;
        if (fRatio <= MAX_RATIO_MARKER)
            uiMarker = static_cast<unsigned char>(static_cast<int>(fRatio));

        pDestination->SetLength(uiCompressedLength + 1);
        pDestination->GetData()[0] = uiMarker;
        return 0;
    }

    unsigned char* pData = pDestination->GetData();
    pDestination->SetLength(rSource.GetLength() + 1);
    pData[0] = STORED_MARKER;
    memcpy(pData + 1, rSource.GetData(), rSource.GetLength());
    return 0;
}

int ZLibCompression::DecompressImpl(const Buffer& rSource, Buffer* pDestination)
{
    const unsigned char* pSource = rSource.GetData();
    unsigned int uiMarker = pSource[0];
    int iStatus = DECOMPRESS_STATUS_OK;
    if (uiMarker != STORED_MARKER)
    {
        bool bFinished = false;
        if (uiMarker == UNKNOWN_RATIO_MARKER)
        {
            pDestination->ResizeByRealSize(rSource.GetLength() * UNKNOWN_RATIO_EXPANSION
                                           + pDestination->GetHeadShiftSize());
        }
        else if (pDestination->GetRealSize() - pDestination->GetHeadShiftSize() < rSource.GetLength() * uiMarker)
        {
            pDestination->ResizeByRealSize(rSource.GetLength() * uiMarker * 2 + pDestination->GetHeadShiftSize());
        }

        unsigned int uiAttempt = 0;
        int iResult;
        do
        {
            quazal_inflateReset(&m_pStreams->m_inflate);
            z_stream* pStream = &m_pStreams->m_inflate;
            pStream->next_out = pDestination->GetData();
            pStream->avail_out = pDestination->GetRealSize() - pDestination->GetHeadShiftSize();
            pStream->avail_in = rSource.GetLength() - 1;
            pStream->next_in = rSource.GetData() + 1;
            iResult = quazal_inflate(pStream, Z_FINISH);
            if ((m_pStreams->m_inflate.avail_out == 0 && iResult == Z_OK) || iResult == Z_BUF_ERROR)
            {
                pDestination->ResizeByRealSize((pDestination->GetRealSize() - pDestination->GetHeadShiftSize()) * 2
                                               + pDestination->GetHeadShiftSize());
            }
            else
            {
                bFinished = true;
            }
            ++uiAttempt;
        } while (!bFinished && uiAttempt < MAX_INFLATE_ATTEMPTS && iResult == Z_OK);

        if (iResult == Z_STREAM_END)
        {
            pDestination->SetLength(m_pStreams->m_inflate.total_out);
        }
        else
        {
            iStatus = DECOMPRESS_STATUS_FAILED;
            if (iResult == Z_OK && !bFinished)
                return DECOMPRESS_STATUS_TRUNCATED;
        }
    }
    else
    {
        pDestination->SetLength(rSource.GetLength() - 1);
        memcpy(pDestination->GetData(), pSource + 1, rSource.GetLength() - 1);
    }

    return iStatus;
}

}
}
