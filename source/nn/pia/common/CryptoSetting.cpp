#include "nn/pia/common/CryptoSetting.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace common
{

CryptoSetting::CryptoSetting() : m_mode(0)
{
    OSBlockSet(m_key, 0, sizeof(m_key));
}

}
}
}
