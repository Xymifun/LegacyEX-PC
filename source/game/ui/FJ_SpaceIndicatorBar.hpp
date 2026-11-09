#ifndef GAME_UI_FJ_SPACEINDICATORBAR_HPP
#define GAME_UI_FJ_SPACEINDICATORBAR_HPP

#include "game/ui/FJ_Base.hpp"

class FJ_SpaceIndicatorBar : public FJ_Base
{
public:
    explicit FJ_SpaceIndicatorBar(fuiRenderNode* renderNode);
    void GetBarFiller();
    void setSaveGameSize(float size);
    void setTotalSize(float size);
    void setSaveGameOffset(float offset);
    static FJ_SpaceIndicatorBar* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void Init(const std::wstring& name);

private:
    float m_barWidth;
    FJ_FuiNode* m_saveGameFiller;
    FJ_FuiNode* m_totalFiller;
    FJ_FuiNode* m_saveBar;
    FJ_FuiNode* m_saveBarFiller;
};

#endif
