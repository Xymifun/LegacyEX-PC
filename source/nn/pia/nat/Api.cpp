#include "nn/pia/nat/Api.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

namespace
{
DebugSetting debugSetting = {};
}

const DebugSetting* GetDebugSetting()
{
    return &debugSetting;
}

}
}
}
