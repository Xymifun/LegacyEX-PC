#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_UPDATE_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_UPDATE_SESSION_SETTING_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class UpdateSessionSetting
{
public:
    enum ModificationFlag
    {
        cModificationFlag_ApplicationData = 1
    };

    UpdateSessionSetting();
    virtual ~UpdateSessionSetting() {}

    virtual Result SetApplicationData(const void* data, unsigned int size) = 0;
    virtual Result GetApplicationData(void* buffer, unsigned int bufferSize) const = 0;
    virtual unsigned int GetApplicationDataSize() const = 0;
    virtual void Trace(unsigned long long value) const;

    virtual unsigned int GetModificationFlag() const;
    virtual bool IsModificationFlagSet(ModificationFlag flag) const;
    virtual bool IsSetApplicationData() const;
    virtual void Reset();

    unsigned int m_modificationFlag;
};

}
}
}

#endif
