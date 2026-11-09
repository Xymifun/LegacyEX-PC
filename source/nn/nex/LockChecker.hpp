#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOCKCHECKER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOCKCHECKER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class LockChecker : public RootObject
{
public:
    explicit LockChecker(unsigned int traceMask);
    LockChecker(const LockChecker& other);
    virtual ~LockChecker();

    LockChecker& operator=(const LockChecker& other);

private:
    bool m_locked;
    unsigned int m_traceMask;
    unsigned int m_ownerThreadId;
    unsigned int m_lockCount;
};

}
}

#endif
