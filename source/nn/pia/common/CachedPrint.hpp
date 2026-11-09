#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CACHED_PRINT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CACHED_PRINT_HPP

#include <cstdarg>
#include "nn/pia.h"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class CachedPrint
{
public:
    explicit CachedPrint(unsigned int capacity);
    ~CachedPrint();
    static Result CreateInstance(unsigned int capacity);
    static void DestroyInstance();
    static CachedPrint* GetInstance() { return s_instance; }
    void VPrintf(const char* format, va_list args);
    void Printf(const char* format, ...);
    void Clear();
    void Flush();
    void Trace(unsigned long long value) const;
    void Read(char* destination, unsigned int* size, unsigned int capacity);

private:
    char* m_buffer;
    unsigned int m_capacity;
    unsigned int m_start;
    unsigned int m_position;
    unsigned int m_end;
    CriticalSection m_criticalSection;
    static CachedPrint* s_instance;
    static unsigned char* s_storage;
};

}
}
}

#endif
