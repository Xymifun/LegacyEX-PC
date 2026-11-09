#ifndef GAME_CORE_SAMPLENOTIFICATIONEVENTHANDLER_HPP
#define GAME_CORE_SAMPLENOTIFICATIONEVENTHANDLER_HPP

#include "nn/nex/NotificationEventHandler.hpp"
#include "nn/nex/NotificationEvent.hpp"

class SampleNotificationEventHandler : public nn::nex::NotificationEventHandler
{
public:
    virtual ~SampleNotificationEventHandler();

    virtual void ProcessNotificationEvent(const nn::nex::NotificationEvent& oEvent);
};

#endif
