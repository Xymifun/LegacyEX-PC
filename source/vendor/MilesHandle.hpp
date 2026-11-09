#ifndef SOURCE_VENDOR_MILES_HANDLE_HPP
#define SOURCE_VENDOR_MILES_HANDLE_HPP

struct HandleEntry
{
    int size;
    unsigned int generation;
    union
    {
        void* data;
        int nextFree;
    };
};

extern int HandleFreeHead;
extern unsigned int HandleGeneration;
extern int HandleBreakOnError;
extern unsigned int HandleCount;
extern unsigned char* HandleTableEnd;
extern unsigned char* HandleHeapStart;
extern unsigned char* HandleHeapCursor;
extern unsigned char* HandleHeapBase;
extern unsigned int HandleHeapSize;
extern int HandleNeedsDefrag;

void Handle_Shutdown();
void Handle_Init(void* pMemory, int iSize);
void Handle_Defrag();
void Handle_MemStats(int* piTotal, int* piFree);

#endif
