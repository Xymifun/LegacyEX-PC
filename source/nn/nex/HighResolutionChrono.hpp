#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCHRONO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCHRONO_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class HighResolutionChrono : public RootObject
{
public:
    explicit HighResolutionChrono(bool bStart);
    virtual ~HighResolutionChrono();
};

}
}

#endif
