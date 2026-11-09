#ifndef GAME_CORE_COMPRESSION_HPP
#define GAME_CORE_COMPRESSION_HPP

#include "game/core/CriticalSection.hpp"
#include "game/core/FileHeader.hpp"

class Compression
{
public:
    enum ECompressionTypes
    {
        eCompressionTypes_None = 0,
        eCompressionTypes_RLE = 1,
        eCompressionTypes_LZXRLE = 2,
        eCompressionTypes_Zlib = 3,
        eCompressionTypes_ZlibPS3 = 4
    };

    class ThreadStorage
    {
    public:
        ThreadStorage();
        ~ThreadStorage();

        Compression* m_pCompression;
    };

    Compression();
    ~Compression();

    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

    static Compression* getCompression();

    int Compress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);
    int Decompress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);
    int DecompressWithType(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);

    int CompressLZXRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize,
                       unsigned int* pRleSize);
    int DecompressLZXRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);

    int CompressRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);
    int DecompressRLE(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);

    static void VitaVirtualDecompress(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);

    void SetDecompressionType(ESavePlatform platform);
    void SetDecompressionType(ECompressionTypes type) { m_decompressionType = type; }

    friend class ConsoleSchematicFile;
    friend class GameRuleManager;

private:
    static int internalCompressRle(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);
    static int internalDecompressRle(void* pDest, unsigned int* pDestSize, void* pSource, unsigned int sourceSize);

    void* m_pCompressionContext;
    void* m_pDecompressionContext;
    OSFastMutex m_compressLock;
    OSFastMutex m_decompressLock;
    unsigned char m_rleBuffer[256 * 1024];
    unsigned char* m_pDecompressBuffer;
    ECompressionTypes m_decompressionType;
    ECompressionTypes m_compressionType;
};

#endif
