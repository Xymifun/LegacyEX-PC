#ifndef SOURCE_GAME_CORE_XMEM_HPP
#define SOURCE_GAME_CORE_XMEM_HPP

enum _XMEMCODEC_TYPE
{
    XMEMCODEC_DEFAULT = 0,
    XMEMCODEC_LZX = 1
};

int XMemCreateCompressionContext(_XMEMCODEC_TYPE, const void*, unsigned int, void**);
int XMemCreateDecompressionContext(_XMEMCODEC_TYPE, const void*, unsigned int, void**);
void XMemDestroyCompressionContext(void*);
void XMemDestroyDecompressionContext(void*);
void* XMemCpy(void* destination, const void* source, unsigned int count);
void* XMemSet(void* destination, int value, unsigned int count);
void* XMemSet128(void* destination, int value, unsigned int count);
void* XPhysicalAlloc(unsigned long size, unsigned long upperAddress, unsigned long alignment, unsigned int protect);
void XPhysicalFree(void* address);

#endif
