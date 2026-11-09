#ifndef GAME_CLIENT_RENDER_PROGRESSRENDERER_HPP
#define GAME_CLIENT_RENDER_PROGRESSRENDERER_HPP

#include <string>

#include "game/network/ProgressListener.hpp"

class Minecraft;

class ProgressRenderer : public ProgressListener
{
public:
    class TextField
    {
    public:
        TextField(bool active, int id, const std::wstring& text) : active(active), id(id), text(text) {}
        TextField(const TextField& other);

        bool active;
        int id;
        std::wstring text;
    };

    ProgressRenderer(Minecraft* minecraft);

    void internalProgressStart(const TextField& field);
    virtual void progressStagePercentage(int percentage);
    virtual void progressStart(int id);
    virtual void progressStartNoAbort(int id);
    virtual void progressStage(int id);
    virtual void progressStage(const std::wstring& text);
    void progressStartNoAbort(const std::wstring& text);
    void progressStart(const std::wstring& text);
    int getCurrentTitle();
    std::wstring getCurrentTitleString();
    std::wstring getCurrentStatusString();
    int getCurrentPercent();
    int getCurrentStatus();

private:
    TextField mStatus;
    Minecraft* mMinecraft;
    TextField mTitle;
    unsigned long long mStartTime;
    bool mNoAbort;
    int mPercent;
};

#endif
