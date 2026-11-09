#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_EVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_EVENT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class EventHandler;

class Event : public RootObject
{
public:
    Event(EventHandler* pEventHandler, unsigned int type, unsigned int context);
    virtual ~Event();

    void Set();
    void Reset();
    bool IsSet();

    void SetType(unsigned int type);
    unsigned int GetType();
    void SetContext(unsigned int context);
    unsigned int GetContext();

    EventHandler* GetEventHandler();
    void* GetPlatformSpecificEvent();

private:
    EventHandler* m_pEventHandler;
    unsigned int m_type;
    unsigned int m_context;
    void* m_pPlatformEvent;
};

}
}

#endif
