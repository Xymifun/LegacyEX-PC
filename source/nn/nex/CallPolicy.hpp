#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CALLPOLICY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CALLPOLICY_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class CallPolicy : public RootObject
{
public:
    virtual ~CallPolicy() {}

    virtual void BeginCallImpl() = 0;
    virtual void EndCallImpl() = 0;
};

}
}

#endif
