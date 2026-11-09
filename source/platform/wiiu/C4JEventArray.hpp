#ifndef SOURCE_PLATFORM_WIIU_C4JEVENTARRAY_HPP
#define SOURCE_PLATFORM_WIIU_C4JEVENTARRAY_HPP

#include <stdint.h>

class C4JEventArray
{
public:
    enum EMode
    {
        Mode0 = 0,
        Mode1 = 1
    };

    virtual ~C4JEventArray() {}
    virtual void Set(int index) = 0;
    virtual void Clear(int index) = 0;
    virtual void SetAll() = 0;
    virtual void ClearAll() = 0;
    virtual unsigned int WaitForAll(int timeout) = 0;
    virtual unsigned int WaitForAny(int timeout) = 0;
    virtual unsigned int WaitForSingle(int index, int timeout) = 0;
    virtual void Cancel() = 0;
};

#endif
