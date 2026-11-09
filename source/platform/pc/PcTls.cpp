#include "platform/wiiu/TLSStorageCafe.hpp"

#include "game/core/CriticalSection.hpp"

namespace
{
const unsigned int TLS_SLOT_COUNT = 64;
const unsigned int TLS_OUT_OF_INDEXES = 0xFFFFFFFF;

OSFastMutex tlsSection;
bool tlsAllocated[TLS_SLOT_COUNT];
__thread void* tlsValues[TLS_SLOT_COUNT];
}

TLSStorageCafe* TLSStorageCafe::m_instance = 0;

TLSStorageCafe::TLSStorageCafe()
{
}

TLSStorageCafe* TLSStorageCafe::Instance()
{
    if (m_instance == 0)
        m_instance = new TLSStorageCafe;
    return m_instance;
}

int TLSStorageCafe::Alloc()
{
    EnterCriticalSection(&tlsSection);
    int found = -1;
    for (unsigned int i = 0; i < TLS_SLOT_COUNT; ++i)
    {
        if (!tlsAllocated[i])
        {
            tlsAllocated[i] = true;
            found = (int)i;
            break;
        }
    }
    LeaveCriticalSection(&tlsSection);
    return found;
}

void TLSStorageCafe::Free(unsigned int i)
{
    if (i < TLS_SLOT_COUNT)
    {
        EnterCriticalSection(&tlsSection);
        tlsAllocated[i] = false;
        LeaveCriticalSection(&tlsSection);
    }
}

unsigned int TLSStorageCafe::GetValue(unsigned int i)
{
    return (unsigned int)(reinterpret_cast<uintptr_t>(TlsGetValue(i)));
}

unsigned int TLSStorageCafe::SetValue(unsigned int i, void* v)
{
    return (unsigned int)TlsSetValue(i, v);
}

unsigned int TlsAlloc()
{
    int i = TLSStorageCafe::Instance()->Alloc();
    return i < 0 ? TLS_OUT_OF_INDEXES : (unsigned int)i;
}

void* TlsGetValue(unsigned int i)
{
    return i < TLS_SLOT_COUNT && tlsAllocated[i] ? tlsValues[i] : 0;
}

int TlsSetValue(unsigned int i, void* v)
{
    if (i >= TLS_SLOT_COUNT || !tlsAllocated[i])
        return 0;
    tlsValues[i] = v;
    return 1;
}
