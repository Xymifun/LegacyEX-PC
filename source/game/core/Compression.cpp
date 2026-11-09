#include "game/core/Compression.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/XMem.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

#include <string.h>
#include <zlib.h>

static unsigned int g_compressionTlsIndex = 0;
static Compression::ThreadStorage* g_pDefaultThreadStorage = NULL;

struct XMemCodecParametersLZX
{
    unsigned int flags;
    unsigned int windowSize;
    unsigned int compressionPartitionSize;
};

static const unsigned int LZX_CODEC_WINDOW_SIZE = 0x20000;

Compression::Compression()
{
    m_pDecompressBuffer = NULL;

    XMemCodecParametersLZX codecParameters;
    codecParameters.flags = 0;
    codecParameters.windowSize = LZX_CODEC_WINDOW_SIZE;
    codecParameters.compressionPartitionSize = LZX_CODEC_WINDOW_SIZE;

    XMemCreateCompressionContext(XMEMCODEC_LZX, &codecParameters, 0, &m_pCompressionContext);
    XMemCreateDecompressionContext(XMEMCODEC_LZX, &codecParameters, 0, &m_pDecompressionContext);

    m_compressionType = eCompressionTypes_Zlib;
    m_decompressionType = eCompressionTypes_Zlib;

    InitializeCriticalSection(&m_compressLock);
    InitializeCriticalSection(&m_decompressLock);
}

Compression::~Compression()
{
    XMemDestroyCompressionContext(m_pCompressionContext);
    XMemDestroyDecompressionContext(m_pDecompressionContext);
    DeleteCriticalSection(&m_compressLock);
    DeleteCriticalSection(&m_decompressLock);

    delete m_pDecompressBuffer;
}

Compression::ThreadStorage::ThreadStorage()
{
    m_pCompression = new Compression();
}

Compression::ThreadStorage::~ThreadStorage()
{
    delete m_pCompression;
}

void Compression::CreateNewThreadStorage()
{
    ThreadStorage* pStorage = new ThreadStorage();

    if (g_pDefaultThreadStorage == NULL)
    {
        g_compressionTlsIndex = TlsAlloc();
        g_pDefaultThreadStorage = pStorage;
    }

    TlsSetValue(g_compressionTlsIndex, pStorage);
}

void Compression::UseDefaultThreadStorage()
{
    TlsSetValue(g_compressionTlsIndex, g_pDefaultThreadStorage);
}

void Compression::ReleaseThreadStorage()
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_compressionTlsIndex));

    if (pStorage != g_pDefaultThreadStorage)
    {
        delete pStorage;
    }
}

Compression* Compression::getCompression()
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_compressionTlsIndex));

    return pStorage->m_pCompression;
}

int Compression::internalCompressRle(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    unsigned char* pInputEnd = (unsigned char*)pSource + sourceSize;
    unsigned char* pOutput = (unsigned char*)pDest;
    unsigned char* pInput = (unsigned char*)pSource;
    unsigned char* pOutputLimit = pOutput + *pDestSize - 3;

    PIXBeginNamedEvent(0.0f, "RLE compression");

    do
    {
        unsigned char value = *pInput++;
        unsigned int runLength = 1;

        while (pInput != pInputEnd && *pInput == value && runLength < 256)
        {
            ++pInput;
            ++runLength;
        }

        if (runLength <= 3)
        {
            if (value == 0xFF)
            {
                *pOutput++ = 0xFF;
                *pOutput++ = (unsigned char)(runLength - 1);
            }
            else
            {
                for (unsigned int i = 0; i < runLength; ++i)
                {
                    *pOutput++ = value;
                }
            }
        }
        else
        {
            *pOutput++ = 0xFF;
            *pOutput++ = (unsigned char)(runLength - 1);
            *pOutput++ = value;
        }
    } while (pInput != pInputEnd && pOutput < pOutputLimit);

    PIXEndNamedEvent();

    *pDestSize = (unsigned int)(pOutput - (unsigned char*)pDest);

    if (pInput != pInputEnd)
    {
        return 0x80CC0001;
    }

    return 0;
}

int Compression::internalDecompressRle(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    unsigned char* pInput = (unsigned char*)pSource;
    unsigned char* pInputEnd = (unsigned char*)pSource + sourceSize;
    unsigned char* pOutput = (unsigned char*)pDest;

    while (pInput != pInputEnd)
    {
        unsigned char value = *pInput++;

        if (value == 0xFF)
        {
            unsigned int runLength = *pInput++;
            unsigned char runValue = 0xFF;

            if (runLength >= 3)
            {
                runValue = *pInput++;
            }

            ++runLength;

            for (unsigned int i = 0; i < runLength; ++i)
            {
                *pOutput++ = runValue;
            }
        }
        else
        {
            *pOutput++ = value;
        }
    }

    *pDestSize = (unsigned int)(pOutput - (unsigned char*)pDest);

    return 0;
}

int Compression::Compress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    uLong compressedSize = *pDestSize;
    int result = 0;

    if (pDest != NULL)
    {
        result = compress((Bytef*)pDest, &compressedSize, (const Bytef*)pSource, sourceSize);

        *pDestSize = compressedSize;
    }
    else
    {
        *pDestSize = compressBound(sourceSize);
    }

    if (result != 0)
    {
        result = -1;
    }

    return result;
}

int Compression::DecompressWithType(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    switch (m_decompressionType)
    {
    case eCompressionTypes_None:
    case eCompressionTypes_RLE:
        memcpy(pDest, pSource, sourceSize);
        *pDestSize = sourceSize;
        return 0;
    default:
        return -1;
    }
}

int Compression::Decompress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    if (m_decompressionType != m_compressionType)
    {
        return DecompressWithType(pDest, pDestSize, pSource, sourceSize);
    }

    EnterCriticalSection(&m_decompressLock);

    uLong decompressedSize = *pDestSize;

    int result = uncompress((Bytef*)pDest, &decompressedSize, (const Bytef*)pSource, sourceSize);

    *pDestSize = decompressedSize;

    LeaveCriticalSection(&m_decompressLock);

    if (result != 0)
    {
        return -1;
    }

    return 0;
}

int Compression::CompressLZXRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize,
                                unsigned int* pRleSize)
{
    EnterCriticalSection(&m_compressLock);

    unsigned int rleSize = sizeof(m_rleBuffer);

    int result = internalCompressRle(m_rleBuffer, &rleSize, pSource, sourceSize);

    if (pRleSize != NULL)
    {
        *pRleSize = rleSize;
    }

    if (result == 0)
    {
        PIXBeginNamedEvent(0.0f, "Secondary compression");
        result = Compress(pDest, pDestSize, m_rleBuffer, rleSize);
        PIXEndNamedEvent();
    }

    LeaveCriticalSection(&m_compressLock);

    return result;
}

int Compression::DecompressLZXRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    EnterCriticalSection(&m_decompressLock);

    unsigned int decompressedSize = *pDestSize;
    unsigned char* pScratchBuffer = NULL;
    unsigned int bufferSize = 200 * 1024;

    if (decompressedSize > bufferSize)
    {
        pScratchBuffer = new unsigned char[decompressedSize];

        Decompress(pScratchBuffer, &decompressedSize, pSource, sourceSize);
        internalDecompressRle(pDest, pDestSize, pScratchBuffer, decompressedSize);
    }
    else
    {
        decompressedSize = bufferSize;

        if (m_pDecompressBuffer == NULL)
        {
            m_pDecompressBuffer = new unsigned char[bufferSize];
        }

        Decompress(m_pDecompressBuffer, &decompressedSize, pSource, sourceSize);
        internalDecompressRle(pDest, pDestSize, m_pDecompressBuffer, decompressedSize);
    }

    if (pScratchBuffer != NULL)
    {
        delete[] pScratchBuffer;
    }

    LeaveCriticalSection(&m_decompressLock);

    return 0;
}

int Compression::CompressRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    EnterCriticalSection(&m_compressLock);

    unsigned int rleSize = sizeof(m_rleBuffer);

    int result = internalCompressRle(m_rleBuffer, &rleSize, pSource, sourceSize);

    if (rleSize > *pDestSize)
    {
        result = 0x80CC0002;
    }
    else if (result == 0)
    {
        *pDestSize = rleSize;
        memcpy(pDest, m_rleBuffer, rleSize);
    }

    LeaveCriticalSection(&m_compressLock);

    return result;
}

int Compression::DecompressRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    EnterCriticalSection(&m_decompressLock);

    internalDecompressRle(pDest, pDestSize, pSource, sourceSize);

    LeaveCriticalSection(&m_decompressLock);

    return 0;
}

void Compression::VitaVirtualDecompress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize)
{
    unsigned char* pOutput = (unsigned char*)pDest;
    unsigned char* pInput = (unsigned char*)pSource;
    unsigned int outputSize = 0;
    unsigned int inputOffset = 0;

    while (inputOffset != sourceSize)
    {
        unsigned char value = pInput[inputOffset];

        if (value != 0)
        {
            pOutput[outputSize] = value;
            ++outputSize;
        }
        else
        {
            ++inputOffset;

            int zeroCount = pInput[inputOffset];

            for (int i = 0; i < zeroCount; ++i)
            {
                pOutput[outputSize] = 0;
                ++outputSize;
            }
        }

        ++inputOffset;
    }

    *pDestSize = outputSize;
}

void Compression::SetDecompressionType(ESavePlatform platform)
{
    switch (platform)
    {
    case eSavePlatform_X360:
        getCompression()->SetDecompressionType(eCompressionTypes_LZXRLE);
        break;
    case eSavePlatform_PS3:
        getCompression()->SetDecompressionType(eCompressionTypes_ZlibPS3);
        break;
    case eSavePlatform_WIIU:
    case eSavePlatform_NX:
    case eSavePlatform_XB1:
    case eSavePlatform_PS4:
    case eSavePlatform_WIN:
    case eSavePlatform_PSV:
        getCompression()->SetDecompressionType(eCompressionTypes_Zlib);
        break;
    }
}
