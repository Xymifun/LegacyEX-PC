#ifndef GAME_UI_FJ_TOOLTIPS_HPP
#define GAME_UI_FJ_TOOLTIPS_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class FJ_ToolTip;
class fuiRenderNode;

class FJ_ToolTips : public FJ_Document
{
public:
    FJ_ToolTips(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    void UpdateControllerType(int controllerType);
    virtual void keyDownHandler(FJ_Event* event);
    void SetToolTip(int index, int type, int subtype, const std::wstring& text, bool visible);
    void UpdateLayout();
    void SetAlpha(float alpha);
    void AnimateToolTip(int index);
    void SetOpacity(float opacity);
    void SetABSwap(bool abSwap);

private:
    std::vector<FJ_FuiNode*> m_toolTipNodes;
    std::vector<FJ_FuiNode*> m_touchNodes;
    unsigned char m_useTouchControls;
    std::wstring m_toolTipSuffix;
    int m_toolTipIndex;
    bool m_touchControlsAvailable;
};

#endif
