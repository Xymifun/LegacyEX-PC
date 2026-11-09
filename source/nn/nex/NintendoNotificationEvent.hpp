#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDONOTIFICATIONEVENT_HPP

#include "nn/nex/_DDL_NintendoNotificationEvent.hpp"

namespace nn
{
namespace nex
{

class NintendoNotificationEvent : public _DDL_NintendoNotificationEvent
{
public:
    NintendoNotificationEvent() : _DDL_NintendoNotificationEvent() {}

    virtual ~NintendoNotificationEvent() {}
};

}
}

#endif
