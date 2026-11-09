#ifndef GAME_UI_UICONTROL_PROGRESS_HPP
#define GAME_UI_UICONTROL_PROGRESS_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_Progress : public UIControl_Base
{
public:
    UIControl_Progress();
    virtual ~UIControl_Progress() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void init(UIString label, int controlId, int minimum, int maximum, int progress);
    void setProgress(int progress);
    void showBar(bool show);
    virtual void ReInit();
    unsigned int getMax();
    void setMax(int maximum);

private:
    int m_minimum;
    int m_maximum;
    int m_progress;
    float m_progressRatio;
    bool m_showBar;
};

#endif
