#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTFILTERINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTFILTERINTERFACE_HPP

#include <stdint.h>

namespace nn
{
namespace nex
{

class NotificationEventFilterInterface
{
public:
    virtual ~NotificationEventFilterInterface() {}

    virtual void SetEventParam1(uint32_t uiParam1) = 0;
    virtual void SetEventUInt64Param1(uint64_t ui64Param1) = 0;
};

}
}

#endif
