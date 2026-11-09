#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_REFCOUNTEDOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_REFCOUNTEDOBJECT_HPP

#include <stdint.h>

#ifdef __ghs__
#include <ppc_ghs.h>
#else
#include <coreinit/atomic.h>
#endif

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class RefCountedObject : public RootObject
{
public:
    RefCountedObject();
    virtual ~RefCountedObject();

    void AddRef() { AddAndFetch(&m_refCount, 1); }
    void ReleaseRef();

    uint32_t GetRefCount() const { return static_cast<uint32_t>(m_refCount); }

private:
    static int32_t AddAndFetch(volatile int32_t* pValue, int32_t iDelta)
    {
#ifdef __ghs__
        unsigned int uiPrevious;
        do
        {
            unsigned int* pWord = reinterpret_cast<unsigned int*>(const_cast<int32_t*>(pValue));
            uiPrevious = __LWARX(pWord, 0);
            __DCBST(pWord, 0);
        } while (!__STWCX(reinterpret_cast<unsigned int*>(const_cast<int32_t*>(pValue)), 0, uiPrevious + iDelta));

        return static_cast<int32_t>(uiPrevious + iDelta);
#elif defined(__powerpc__) || defined(__PPC__)
        int32_t iPrevious;
        int32_t iNext;
        __asm__ __volatile__("1:\n\tlwarx %0, 0, %2\n\tdcbst 0, %2\n\tadd %1, %0, %3\n\tstwcx. %1, 0, %2\n\tbne- 1b"
                             : "=&r"(iPrevious), "=&r"(iNext)
                             : "r"(pValue), "r"(iDelta)
                             : "cr0", "memory");
        return iNext;
#else
        return OSAddAtomic(pValue, iDelta) + iDelta;
#endif
    }

    uint32_t m_unknown0;
    volatile int32_t m_refCount;
    bool m_disposing;
};

}
}

#endif
