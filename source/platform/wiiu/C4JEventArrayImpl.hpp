#ifndef SOURCE_PLATFORM_WIIU_C4JEVENTARRAYIMPL_HPP
#define SOURCE_PLATFORM_WIIU_C4JEVENTARRAYIMPL_HPP

#include <stdint.h>
#include "platform/wiiu/C4JEventArray.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"

class C4JEventArrayImpl : public C4JEventArray
{
public:
    C4JEventArrayImpl(int count, C4JEventArray::EMode mode);
    virtual ~C4JEventArrayImpl();
    virtual void Set(int index);
    virtual void Clear(int index);
    virtual void SetAll();
    virtual void ClearAll();
    virtual unsigned int WaitForAll(int timeout);
    virtual unsigned int WaitForAny(int timeout);
    virtual unsigned int WaitForSingle(int index, int timeout);
    virtual void Cancel();

private:
    int m_count;
    C4JEventArray::EMode m_mode;
    OSEvent* m_events;
};

#endif
