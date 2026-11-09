#include "nn/nex/CriticalSection.hpp"

namespace nn
{
namespace nex
{

bool g_criticalSectionDisabled = false;

CriticalSection::CriticalSection(unsigned int traceMask) : m_mutex(), m_lockChecker(traceMask)
{
}

CriticalSection::CriticalSection(const CriticalSection& other) : m_mutex(), m_lockChecker(other.m_lockChecker)
{
}

CriticalSection::~CriticalSection()
{
}

bool CriticalSection::IsForce() const
{
    return false;
}

}
}
