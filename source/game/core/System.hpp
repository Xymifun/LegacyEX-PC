#ifndef GAME_CORE_SYSTEM_HPP
#define GAME_CORE_SYSTEM_HPP

#include <cstring>
#include <stdint.h>

#include "game/world/biome/arrayWithLength.hpp"

struct Node;
class Biome;

class System
{
public:
    enum ETimeGranularity
    {
        eTimeGranularity_MilliSecs,
        eTimeGranularity_MicroSecs,
        eTimeGranularity_NanoSecs
    };

    template <class T>
    static void arraycopy(arrayWithLength<T> src, unsigned int srcPos, arrayWithLength<T>* dest, unsigned int destPos,
                          unsigned int length);
    static void arraycopy(arrayWithLength<unsigned char> src, unsigned int srcPos, arrayWithLength<unsigned char>* dest,
                          unsigned int destPos, unsigned int length);
    static void arraycopy(arrayWithLength<Node*> src, unsigned int srcPos, arrayWithLength<Node*>* dest,
                          unsigned int destPos, unsigned int length);
    static void arraycopy(arrayWithLength<Biome*> src, unsigned int srcPos, arrayWithLength<Biome*>* dest,
                          unsigned int destPos, unsigned int length);
    static void arraycopy(arrayWithLength<int> src, unsigned int srcPos, arrayWithLength<int>* dest,
                          unsigned int destPos, unsigned int length);
    static void arraycopy(arrayWithLength<unsigned short> src, unsigned int srcPos,
                          arrayWithLength<unsigned short>* dest, unsigned int destPos, unsigned int length);

    static uint64_t processTime(ETimeGranularity granularity);
    static uint64_t processTimeInSeconds();
    static long long processTimeInMilliSecs();
    static unsigned int processTimeInMicroSecs();
    static uint64_t processTimeInNanoSecs();
    static uint64_t currentRealTimeMillis();
    static uint64_t currentUnixEpochTimeSeconds();
    static void ReverseUSHORT(unsigned short* value);
    static void ReverseSHORT(short* value);
    static void ReverseULONG(unsigned int* value);
    static void ReverseULONG(unsigned long* value);
    static void ReverseINT(int* value);
    static void ReverseULONGLONG(long long* value);
    static void ReverseCHAR16A(wchar_t* text, int count);
};

template <class T>
void System::arraycopy(arrayWithLength<T> src, unsigned int srcPos, arrayWithLength<T>* dest, unsigned int destPos,
                       unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(T));
}

#endif
