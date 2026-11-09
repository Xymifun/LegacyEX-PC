#include "nn/nex/LockChecker.hpp"

namespace nn
{
namespace nex
{

LockChecker::LockChecker(unsigned int traceMask)
    : m_locked(false),
      m_traceMask(traceMask),
      m_ownerThreadId(0),
      m_lockCount(0)
{
}

LockChecker::LockChecker(const LockChecker& other)
    : m_locked(false),
      m_traceMask(other.m_traceMask),
      m_ownerThreadId(0),
      m_lockCount(0)
{
}

LockChecker::~LockChecker()
{
}

}
}
