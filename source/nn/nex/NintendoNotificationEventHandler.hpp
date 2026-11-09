#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENTHANDLER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class NintendoNotificationEvent;

class NintendoNotificationEventHandler : public RootObject
{
public:
    virtual ~NintendoNotificationEventHandler() {}

    virtual void ProcessNintendoNotificationEvent(const NintendoNotificationEvent& rEvent) = 0;
};

}
}

#endif
