#ifndef GAME_UI_FJ_CONTROLSMENU_HPP
#define GAME_UI_FJ_CONTROLSMENU_HPP

#include <string>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_ControlsMenu : public FJ_Document
{
public:
    FJ_ControlsMenu(fuiRenderNode* renderNode);

    void handleCheckboxToggled(FJ_Event* event);
    static void _handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void init(FJ_Event* event);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void PositionControlsHelp();
    void SetControlsHelp(int id, const std::wstring& help);
};

#endif
