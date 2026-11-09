#include "nn/pia/session/UpdateSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace session
{

UpdateSessionSetting::UpdateSessionSetting() : m_modificationFlag(0)
{
}

void UpdateSessionSetting::Reset()
{
    m_modificationFlag = 0;
}

void UpdateSessionSetting::Trace(unsigned long long value) const
{
}

unsigned int UpdateSessionSetting::GetModificationFlag() const
{
    return m_modificationFlag;
}

bool UpdateSessionSetting::IsModificationFlagSet(ModificationFlag flag) const
{
    return ((m_modificationFlag >> flag) & 1) != 0;
}

bool UpdateSessionSetting::IsSetApplicationData() const
{
    return IsModificationFlagSet(cModificationFlag_ApplicationData);
}

}
}
}
