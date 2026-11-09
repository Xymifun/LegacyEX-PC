#ifndef GAME_UI_FJ_INGAMEPLAYEROPTIONS_HPP
#define GAME_UI_FJ_INGAMEPLAYEROPTIONS_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_InGamePlayerOptions : public FJ_Document
{
public:
    FJ_InGamePlayerOptions(fuiRenderNode* renderNode);

    void handleCheckboxToggled(FJ_Event* event);
    static void _handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void InitMenu();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void SetPlayerIcon(int icon);
};

#endif
