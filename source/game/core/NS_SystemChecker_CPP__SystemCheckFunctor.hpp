#ifndef GAME_CORE_NS_SYSTEMCHECKER_CPP__SYSTEMCHECKFUNCTOR_HPP
#define GAME_CORE_NS_SYSTEMCHECKER_CPP__SYSTEMCHECKFUNCTOR_HPP

#include "game/core/NS_SystemChecker_CPP__VoidFunctor.hpp"

namespace NS_SystemChecker_CPP
{
class SystemCheckFunctor : public VoidFunctor
{
public:
    typedef void (*Handler)(const wchar_t* pExpression, const wchar_t* pFile, unsigned int uiLine);

    SystemCheckFunctor(Handler pHandler, const wchar_t* pExpression, const wchar_t* pFile, unsigned int uiLine)
        : m_pHandler(pHandler),
          m_pExpression(pExpression),
          m_pFile(pFile),
          m_uiLine(uiLine)
    {
    }

    virtual ~SystemCheckFunctor();

    virtual void operator()() const;

private:
    Handler m_pHandler;
    const wchar_t* m_pExpression;
    const wchar_t* m_pFile;
    unsigned int m_uiLine;
};
}

#endif
