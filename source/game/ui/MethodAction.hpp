#ifndef GAME_UI_METHODACTION_HPP
#define GAME_UI_METHODACTION_HPP

#include "game/core/TickAction.hpp"

template <class T>
class MethodAction : public TickAction
{
public:
    typedef void (T::*Func)();

    MethodAction(T* pTarget, Func pFunc) : m_pTarget(pTarget), m_pFunc(pFunc) {}
    virtual ~MethodAction() {}

    virtual void Execute() { (m_pTarget->*m_pFunc)(); }

private:
    T* m_pTarget;
    Func m_pFunc;
};

#endif
