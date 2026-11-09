#ifndef GAME_UI_UICONTROL_SPACEINDICATORBAR_HPP
#define GAME_UI_UICONTROL_SPACEINDICATORBAR_HPP

#include <utility>
#include <vector>
#include "game/ui/UIControl_Base.hpp"

class UIControl_SpaceIndicatorBar : public UIControl_Base
{
public:
    UIControl_SpaceIndicatorBar();
    virtual ~UIControl_SpaceIndicatorBar() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void setSaveSize(long long size);
    void setSaveGameOffset(float offset);
    void selectSave(int index);
    void setTotalSize(long long size);
    void reset();
    void addSave(long long size);
    void init(UIString label, int controlId, long long startOffset, long long totalOffset);
    virtual void ReInit();

private:
    long long m_startOffset;
    long long m_endOffset;
    long long m_saveSize;
    long long m_totalSize;
    float m_saveGameOffset;
    std::vector<std::pair<long long, float> > m_saves;
};

#endif
