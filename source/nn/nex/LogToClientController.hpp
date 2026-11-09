#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGTOCLIENTCONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGTOCLIENTCONTROLLER_HPP

#include "nn/nex/LogDevice.hpp"

namespace nn
{
namespace nex
{

class EndPoint;

class LogToClientController
{
public:
    class LogDeviceInterface : public LogDevice
    {
    public:
        LogDeviceInterface();
        virtual ~LogDeviceInterface();

        virtual void Accumulate() = 0;
        virtual void Flush(EndPoint* pEndPoint) = 0;
        virtual bool CanFlush() = 0;

        static LogDeviceInterface* GetInstance();

    private:
        static LogDeviceInterface* s_pInstance;
    };

    static void Accumulate();
    static void Flush(EndPoint* pEndPoint);
};

}
}

#endif
