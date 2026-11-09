#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/util/Vec3.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

#include <coreinit/messagequeue.h>
#include <coreinit/thread.h>
#include <cstddef>
#include <cstdio>

extern "C" int ProcUIProcessMessages(int count);
void ShutdownAndExit();
void ReleaseCallback();

extern CInput* g_pInput;
extern Renderer* gRenderer;
extern CConsoleMinecraftApp gApp;
extern unsigned char s_altMainOKToRun;
extern unsigned char s_altMainPending;
extern unsigned char s_altMainExit;
extern OSMessageQueue gMainMessageQueue;

extern OSFastMutex gMainMutex;
unsigned char s_altMainStopRequested;

void altMain()
{
    OSMessage message;
    s_altMainPending = 1;
    OSSendMessage(&gMainMessageQueue, &message, OS_MESSAGE_FLAGS_NONE);
}

void altMainStop()
{
    s_altMainStopRequested = 0;
}

void waitForAltMainThreadToComplete()
{
    while (s_altMainStopRequested != 0)
        Sleep(10);
}

int altMainThread(int, void*)
{
    OSMessage message;
    while (!s_altMainExit)
    {
        gRenderer->InitialiseContext();
        OSReceiveMessage(&gMainMessageQueue, &message, OS_MESSAGE_FLAGS_BLOCKING);
        while (s_altMainOKToRun == 0)
        {
            Sleep(10);
        }

        EnterCriticalSection(&gMainMutex);
        s_altMainStopRequested = 1;
        Sleep(200);
        s_altMainPending = 0;
        s_altMainOKToRun = 0;

        while (s_altMainStopRequested != 0)
        {
            int processResult = ProcUIProcessMessages(1);
            if (processResult == 3)
            {
                s_altMainExit = 1;
                LeaveCriticalSection(&gMainMutex);
                ShutdownAndExit();
                break;
            }
            if (processResult == 2)
            {
                ReleaseCallback();
            }

            gRenderer->StartFrame();
            gApp.UpdateTime();

            PIXBeginNamedEvent(0.0f, "Input manager tick");
            g_pInput->Tick();
            PIXEndNamedEvent();

            PIXBeginNamedEvent(0.0f, "Render manager tick");
            gRenderer->Tick();
            PIXEndNamedEvent();

            PIXBeginNamedEvent(0.0f, "Network manager do work #1");
            CGameNetworkManager::sInstance.DoWork();
            PIXEndNamedEvent();

            gRenderer->ResetRenderState();
            const float clearColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            gRenderer->SetClearColour(clearColour);
            gRenderer->Clear(3);
            gRenderer->StartDRCFrame(false);
            gRenderer->Clear(3);

            PIXBeginNamedEvent(0.0f, "Frame present");
            gRenderer->Present();
            PIXEndNamedEvent();

            gRenderer->Set_matrixDirty();
            Vec3::resetPool();
        }

        LeaveCriticalSection(&gMainMutex);
    }

    return 0;
}

#if !defined(RECON_NATIVE)
int _itoa_s(int value, char* buffer, unsigned int, int radix)
{
    int result;
    if (radix == 10)
    {
        std::sprintf(buffer, "%d", value);
        result = 0;
    }
    else if (radix == 16)
    {
        std::sprintf(buffer, "%x", value);
        result = 0;
    }
    else
    {
        result = -1;
    }
    return result;
}

int _i64toa_s(long long value, char* buffer, unsigned int, int radix)
{
    if (radix == 10)
    {
        std::sprintf(buffer, "%lld", value);
        return 0;
    }
    return -1;
}
#endif
