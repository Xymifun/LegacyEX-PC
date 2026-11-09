#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_INSTANTIATIONCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_INSTANTIATIONCONTEXT_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/SystemError.hpp"

namespace nn
{
namespace nex
{

class InstanceControl;

class InstantiationContext : public RootObject
{
public:
    enum
    {
        NumInstances = 15
    };

    InstantiationContext();
    InstantiationContext(InstanceControl* instance, unsigned int classId);
    virtual ~InstantiationContext();

    bool AddInstance(InstanceControl* instance, unsigned int classId);
    bool DelInstance(InstanceControl* instance, unsigned int classId);

    InstanceControl* GetInstance(unsigned int classId) const
    {
        if (classId >= NumInstances)
        {
            SystemError::SignalError(
                L"D:\\WiiU\\Cafe_SDK_2.13.01\\SDK\\nex\\include\\OnlineCore/src/Core/InstantiationContext.h", 0x53,
                0xE0000003, 0);
            return reinterpret_cast<InstanceControl*>(0xFFFFFFFF);
        }

        return m_instances[classId];
    }

private:
    friend class InstanceTable;

    void InitContext();

    InstanceControl* m_instances[NumInstances];
    bool m_initialized;
};

}
}

#endif
