#include "game/core/System.hpp"
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

void System::arraycopy(arrayWithLength<unsigned char> src, unsigned int srcPos, arrayWithLength<unsigned char>* dest,
                       unsigned int destPos, unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length);
}

void System::arraycopy(arrayWithLength<Node*> src, unsigned int srcPos, arrayWithLength<Node*>* dest,
                       unsigned int destPos, unsigned int length)
{
    System::arraycopy<Node*>(src, srcPos, dest, destPos, length);
}

void System::arraycopy(arrayWithLength<Biome*> src, unsigned int srcPos, arrayWithLength<Biome*>* dest,
                       unsigned int destPos, unsigned int length)
{
    System::arraycopy<Biome*>(src, srcPos, dest, destPos, length);
}

void System::arraycopy(arrayWithLength<int> src, unsigned int srcPos, arrayWithLength<int>* dest, unsigned int destPos,
                       unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(int));
}

void System::arraycopy(arrayWithLength<unsigned short> src, unsigned int srcPos, arrayWithLength<unsigned short>* dest,
                       unsigned int destPos, unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(unsigned short));
}

uint64_t System::processTime(ETimeGranularity granularity)
{
    switch (granularity)
    {
    case eTimeGranularity_MilliSecs:
        return (unsigned long long)OSGetTime() * 1000 / (OSGetSystemInfo()->busClockSpeed >> 2);
    case eTimeGranularity_MicroSecs:
        return (unsigned long long)OSGetTime() * 1000000 / (OSGetSystemInfo()->busClockSpeed >> 2);
    case eTimeGranularity_NanoSecs:
        return (unsigned long long)OSGetTime() * 32000
               / ((unsigned long long)(OSGetSystemInfo()->busClockSpeed >> 2) / 31250);
    }
    return 0;
}

uint64_t System::processTimeInSeconds()
{
    return (long long)processTime(eTimeGranularity_MilliSecs) / 1000;
}

long long System::processTimeInMilliSecs()
{
    return processTime(eTimeGranularity_MilliSecs);
}

uint64_t System::processTimeInNanoSecs()
{
    return processTime(eTimeGranularity_NanoSecs);
}

uint64_t System::currentRealTimeMillis()
{
    return (unsigned long long)OSGetTime() * 1000 / (OSGetSystemInfo()->busClockSpeed >> 2);
}

uint64_t System::currentUnixEpochTimeSeconds()
{
    return (long long)currentRealTimeMillis() / 1000;
}

void System::ReverseUSHORT(unsigned short* value)
{
    unsigned short original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;
    destination[0] = source[1];
    destination[1] = source[0];
}

void System::ReverseSHORT(short* value)
{
    unsigned short original = (unsigned short)*value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;
    destination[0] = source[1];
    destination[1] = source[0];
}

void System::ReverseULONG(unsigned int* value)
{
    unsigned int original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;
    destination[0] = source[3];
    destination[1] = source[2];
    destination[2] = source[1];
    destination[3] = source[0];
}

void System::ReverseINT(int* value)
{
    unsigned int original = (unsigned int)*value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;
    destination[0] = source[3];
    destination[1] = source[2];
    destination[2] = source[1];
    destination[3] = source[0];
}

void System::ReverseULONGLONG(long long* value)
{
    unsigned long long original = (unsigned long long)*value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;
    destination[0] = source[7];
    destination[1] = source[6];
    destination[2] = source[5];
    destination[3] = source[4];
    destination[4] = source[3];
    destination[5] = source[2];
    destination[6] = source[1];
    destination[7] = source[0];
}

void System::ReverseCHAR16A(wchar_t* text, int count)
{
    for (int i = 0; i < count; i++)
    {
        ReverseUSHORT((unsigned short*)&text[i]);
    }
}
