#ifndef GAME_UI_FJ_PROGRESSBAR_HPP
#define GAME_UI_FJ_PROGRESSBAR_HPP

#include "game/ui/FJ_Base.hpp"

class FJ_ProgressBar : public FJ_Base
{
public:
    explicit FJ_ProgressBar(fuiRenderNode* renderNode);
    virtual ~FJ_ProgressBar() {}

    virtual void InitBarType();
    virtual void GetBarFiller();
    virtual void Init(const std::wstring& label);
    virtual void setProgress(float progress);
    void initFiller();
    void ShowBar(bool show);
    float getProgress();

protected:
    float m_progress;
    float m_fillerX;
    float m_fillerY;
    float m_fillerWidth;
    float m_fillerHeight;
    FJ_FuiNode* m_pBarFiller;
    FJ_FuiNode* m_pFiller;
    bool m_barType;
    bool m_reverse;
};

#endif
