#include "nn/pia/common/Trace.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
enum
{
    cResultAllocationFailed = 0x10c01,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultAlreadyExists = 0x10c0f
};

const unsigned long long cDefaultTraceFlags = 1;
}

Trace* Trace::s_instance = 0;
unsigned char* Trace::s_storage = 0;

Trace::Trace() : m_flags(cDefaultTraceFlags), m_start(), m_enabled(true)
{
    m_start.SetNow();
}

Result Trace::CreateInstance()
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (s_instance != 0)
        return cResultAlreadyExists;

    s_storage = NewArray<unsigned char>(sizeof(Trace), 8);
    if (s_storage == 0)
        return cResultAllocationFailed;
    s_instance = new (s_storage) Trace();
    return 0;
}

void Trace::DestroyInstance()
{
    if (s_instance != 0)
        s_instance = 0;
    if (s_storage != 0)
    {
        DeleteArray(s_storage);
        s_storage = 0;
    }
}

}
}
}
