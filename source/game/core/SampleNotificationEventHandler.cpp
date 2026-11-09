#include "game/core/SampleNotificationEventHandler.hpp"

#include "nn/nex/StringStream.hpp"

static unsigned int s_uiNewOwnerPid = 0;

SampleNotificationEventHandler::~SampleNotificationEventHandler()
{
}

void SampleNotificationEventHandler::ProcessNotificationEvent(const nn::nex::NotificationEvent& oEvent)
{
    nn::nex::StringStream ssEvent;
    ssEvent << oEvent;

    if (oEvent.GetType() / 1000 == 4)
    {
        s_uiNewOwnerPid = oEvent.GetParam2();
    }
}
