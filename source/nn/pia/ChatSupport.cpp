#include "nn/pia/ChatSupport.hpp"

#include <nn/middleware.h>

namespace nn
{
namespace pia
{
namespace chat
{

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_CHAT_5_9_0]");
}

}
}
}

namespace nn
{
namespace pia
{
namespace common
{
void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_COMMON_5_9_0]");
}
}
}
}
