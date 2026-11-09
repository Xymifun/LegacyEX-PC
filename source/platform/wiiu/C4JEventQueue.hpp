#ifndef SOURCE_PLATFORM_WIIU_C4JEVENTQUEUE_HPP
#define SOURCE_PLATFORM_WIIU_C4JEVENTQUEUE_HPP

class C4JEventQueue
{
public:
    virtual ~C4JEventQueue() {}
    virtual void setProcessor(int processor) = 0;
    virtual void setPriority(int priority) = 0;
    virtual void sendEvent(void* event) = 0;
    virtual void waitForFinish() = 0;
};

#endif
