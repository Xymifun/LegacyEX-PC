#ifndef GAME_UI_FJ_MULTILISTMENU_HPP
#define GAME_UI_FJ_MULTILISTMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_MultilistMenu : public FJ_Document
{
public:
    explicit FJ_MultilistMenu(fuiRenderNode* renderNode);

    void handleCheckboxToggled(FJ_Event* event);
    static void _handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void DoVerticalResizeCheck();
    virtual void DoHorizontalResizeCheck();
    void HideDescription();
};

#endif
