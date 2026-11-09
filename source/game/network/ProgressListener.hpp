#ifndef GAME_NETWORK_PROGRESSLISTENER_HPP
#define GAME_NETWORK_PROGRESSLISTENER_HPP

#include <string>

class ProgressListener
{
public:
    virtual ~ProgressListener() {}

    virtual void progressStagePercentage(int percentage) = 0;
    virtual void progressStart(int titleId) = 0;
    virtual void progressStartNoAbort(int titleId) = 0;
    virtual void progressStage(int statusId) = 0;
    virtual void progressStage(const std::wstring& status) = 0;
};

#endif
