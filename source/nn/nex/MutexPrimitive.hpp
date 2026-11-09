#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MUTEXPRIMITIVE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MUTEXPRIMITIVE_HPP

#include <coreinit/mutex.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class MutexPrimitive : public RootObject
{
public:
    MutexPrimitive();
    virtual ~MutexPrimitive();

    void Lock() { OSLockMutex(&m_mutex); }

    void Unlock() { OSUnlockMutex(&m_mutex); }

private:
    friend class Condition;

    OSMutex m_mutex;
};

}
}

#endif
