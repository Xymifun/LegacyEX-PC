#include "game/client/render/ProgressRenderer.hpp"

#include <cwchar>

#include "game/core/CriticalSection.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"

static OSFastMutex sProgressRendererMutex;

ProgressRenderer::TextField::TextField(const TextField& other) : active(other.active), id(other.id), text(other.text)
{
}

ProgressRenderer::ProgressRenderer(Minecraft* minecraft)
    : mStatus(false, -1, std::wstring(L"")),
      mTitle(false, -1, std::wstring(L""))
{
    mStartTime = static_cast<unsigned long long>(System::processTimeInMilliSecs());
    mNoAbort = false;
    mMinecraft = minecraft;
}

void ProgressRenderer::internalProgressStart(const TextField& field)
{
    if (mMinecraft->mRunning || !mNoAbort)
    {
        EnterCriticalSection(&sProgressRendererMutex);
        mPercent = 0;
        if (!field.active)
        {
            mTitle = field;
        }
        else if (field.id == -2)
        {
            mTitle.active = field.active;
            mTitle.id = -2;
        }
        else
        {
            mTitle = field;
        }
        LeaveCriticalSection(&sProgressRendererMutex);
    }
}

void ProgressRenderer::progressStart(int id)
{
    mNoAbort = false;
    internalProgressStart(TextField(true, id, std::wstring(L10N::GetString(id))));
}

void ProgressRenderer::progressStartNoAbort(int id)
{
    mNoAbort = true;
    internalProgressStart(TextField(true, id, std::wstring(L10N::GetString(id))));
}

void ProgressRenderer::progressStartNoAbort(const std::wstring& text)
{
    mNoAbort = true;
    internalProgressStart(TextField(false, -1, text));
}

void ProgressRenderer::progressStart(const std::wstring& text)
{
    mNoAbort = false;
    internalProgressStart(TextField(false, -1, text));
}

void ProgressRenderer::progressStage(int id)
{
    if (mMinecraft->mRunning || !mNoAbort)
    {
        mStartTime = 0;
        EnterCriticalSection(&sProgressRendererMutex);
        mStatus = TextField(true, id, std::wstring(L10N::GetString(id)));
        LeaveCriticalSection(&sProgressRendererMutex);
        progressStagePercentage(-1);
        mStartTime = 0;
    }
}

void ProgressRenderer::progressStage(const std::wstring& text)
{
    if (mMinecraft->mRunning || !mNoAbort)
    {
        mStartTime = 0;
        EnterCriticalSection(&sProgressRendererMutex);
        mStatus = TextField(false, -1, text);
        LeaveCriticalSection(&sProgressRendererMutex);
        progressStagePercentage(-1);
        mStartTime = 0;
    }
}

void ProgressRenderer::progressStagePercentage(int percentage)
{
    EnterCriticalSection(&sProgressRendererMutex);
    mPercent = percentage;
    LeaveCriticalSection(&sProgressRendererMutex);
}

int ProgressRenderer::getCurrentTitle()
{
    EnterCriticalSection(&sProgressRendererMutex);
    TextField field(mTitle);
    LeaveCriticalSection(&sProgressRendererMutex);
    return field.active ? field.id : -1;
}

std::wstring ProgressRenderer::getCurrentTitleString()
{
    EnterCriticalSection(&sProgressRendererMutex);
    std::wstring result(mTitle.text);
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}

std::wstring ProgressRenderer::getCurrentStatusString()
{
    EnterCriticalSection(&sProgressRendererMutex);
    std::wstring result(mStatus.text);
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}

int ProgressRenderer::getCurrentPercent()
{
    EnterCriticalSection(&sProgressRendererMutex);
    int result = mPercent;
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}

int ProgressRenderer::getCurrentStatus()
{
    EnterCriticalSection(&sProgressRendererMutex);
    int result = mStatus.active ? mStatus.id : -1;
    LeaveCriticalSection(&sProgressRendererMutex);
    return result;
}
