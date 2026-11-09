#include "platform/wiiu/TLSStorageCafe.hpp"

#include <coreinit/thread.h>

static const unsigned int TLS_SLOT_COUNT = 64;

static unsigned int s_tlsAllocated[TLS_SLOT_COUNT];
#if defined(__ghs__) || defined(RECON_NATIVE)
static __thread void* s_tlsValues[TLS_SLOT_COUNT];
#else
static const OSThreadSpecificID TLS_VALUES_SLOT = OS_THREAD_SPECIFIC_13;

class ThreadTlsValues
{
public:
    void*& operator[](unsigned int index)
    {
        void** values = static_cast<void**>(OSGetThreadSpecific(TLS_VALUES_SLOT));
        if (values == 0)
        {
            values = new void*[TLS_SLOT_COUNT]();
            OSSetThreadSpecific(TLS_VALUES_SLOT, values);
        }
        return values[index];
    }
};

static ThreadTlsValues s_tlsValues;
#endif

TLSStorageCafe* TLSStorageCafe::m_instance = 0;

TLSStorageCafe::TLSStorageCafe()
{
    unsigned int index;
    for (index = 0; index < TLS_SLOT_COUNT; ++index)
    {
        s_tlsAllocated[index] = 0;
        s_tlsValues[index] = 0;
    }
}

int TLSStorageCafe::Alloc()
{
    unsigned int index;
    for (index = 0; index < TLS_SLOT_COUNT; ++index)
    {
        if (s_tlsAllocated[index] == 0)
        {
            s_tlsAllocated[index] = 1;
            s_tlsValues[index] = 0;
            return (int)index;
        }
    }
    return -1;
}

TLSStorageCafe* TLSStorageCafe::Instance()
{
    if (m_instance == 0)
    {
        m_instance = new TLSStorageCafe;
    }
    return m_instance;
}

void TLSStorageCafe::Free(unsigned int index)
{
    if (s_tlsAllocated[index] != 0)
    {
        s_tlsAllocated[index] = 0;
    }
}

unsigned int TLSStorageCafe::GetValue(unsigned int index)
{
    unsigned int value = 0;
    if (s_tlsAllocated[index] != 0)
    {
        value = (unsigned int)s_tlsValues[index];
    }
    return value;
}

unsigned int TLSStorageCafe::SetValue(unsigned int index, void* value)
{
    if (s_tlsAllocated[index] == 0)
    {
        return 0;
    }
    s_tlsValues[index] = value;
    return 1;
}

unsigned int TlsAlloc()
{
    return static_cast<unsigned int>(TLSStorageCafe::Instance()->Alloc());
}

void* TlsGetValue(unsigned int index)
{
    return reinterpret_cast<void*>(TLSStorageCafe::Instance()->GetValue(index));
}

int TlsSetValue(unsigned int index, void* value)
{
    return static_cast<int>(TLSStorageCafe::Instance()->SetValue(index, value));
}
