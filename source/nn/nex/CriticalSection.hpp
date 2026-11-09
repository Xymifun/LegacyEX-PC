#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CRITICALSECTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CRITICALSECTION_HPP

#include "nn/nex/LockChecker.hpp"
#include "nn/nex/MutexPrimitive.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

extern bool g_criticalSectionDisabled;

class CriticalSection : public RootObject
{
public:
    enum
    {
        DEFAULT_TRACE_MASK = 0x20000000
    };

    explicit CriticalSection(unsigned int traceMask = DEFAULT_TRACE_MASK);
    CriticalSection(const CriticalSection& other);
    virtual ~CriticalSection();

    CriticalSection& operator=(const CriticalSection&) { return *this; }

    virtual void Enter()
    {
        if (g_criticalSectionDisabled)
            return;

        m_mutex.Lock();
    }

    virtual void Leave()
    {
        if (g_criticalSectionDisabled)
            return;

        m_mutex.Unlock();
    }

    virtual bool IsForce() const;

    void ValidateOwnership(bool owned) const;
    bool HasOwnership() const;
    bool HasNotOwnership() const;

protected:
    friend class Condition;
    friend class Core;

    MutexPrimitive m_mutex;
    LockChecker m_lockChecker;
};

class ForcedCriticalSection : public CriticalSection
{
public:
    explicit ForcedCriticalSection(unsigned int traceMask) : CriticalSection(traceMask) {}

    virtual ~ForcedCriticalSection() {}

    virtual void Enter() { m_mutex.Lock(); }

    virtual void Leave() { m_mutex.Unlock(); }

    virtual bool IsForce() const { return true; }
};

class ScopedCS : public RootObject
{
public:
    explicit ScopedCS(CriticalSection* criticalSection) : m_criticalSection(criticalSection)
    {
        m_criticalSection->Enter();
    }

    ~ScopedCS() { m_criticalSection->Leave(); }

private:
    CriticalSection* m_criticalSection;
};

}
}

#endif
