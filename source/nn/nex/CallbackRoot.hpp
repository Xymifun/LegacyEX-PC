#ifndef SOURCE_NN_NEX_CALLBACKROOT_HPP
#define SOURCE_NN_NEX_CALLBACKROOT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class CallbackRoot : public RootObject
{
public:
    virtual ~CallbackRoot();

    virtual void Call();
    virtual void CallObjectMethod() = 0;
};

}
}

#endif
