#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CALLBACK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CALLBACK_HPP

#include "nn/nex/CallbackRoot.hpp"

namespace nn
{
namespace nex
{

template <class T, class Argument>
class Callback : public CallbackRoot
{
public:
    typedef void (T::*Method)(Argument);

    Callback(T* pObject, Method pMethod, Argument argument)
        : m_pObject(pObject),
          m_pMethod(pMethod),
          m_argument(argument)
    {
    }

    virtual ~Callback() {}

    virtual void CallObjectMethod() { (m_pObject->*m_pMethod)(m_argument); }

private:
    T* m_pObject;
    Method m_pMethod;
    Argument m_argument;
};

}
}

#endif
