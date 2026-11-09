#include "game/core/NS_SystemChecker_CPP__SystemCheckFunctor.hpp"

#include <cstddef>
#include <nex.h>

namespace NS_SystemChecker_CPP
{

SystemCheckFunctor::~SystemCheckFunctor()
{
}

void SystemCheckFunctor::operator()() const
{
    if (nn::nex::BadEvents::s_pInstance != NULL)
    {
        nn::nex::BadEvents::Signal(nn::nex::BadEvents::SYSTEM_CHECK_FAILURE);
    }

    if (m_pHandler != NULL)
    {
        m_pHandler(m_pExpression, m_pFile, m_uiLine);
    }
}

}
