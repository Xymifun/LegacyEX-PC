#ifndef GAME_UI_FJ_DEBUGCREATESCHEMATIC_HPP
#define GAME_UI_FJ_DEBUGCREATESCHEMATIC_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_DebugCreateSchematic : public FJ_Document
{
public:
    FJ_DebugCreateSchematic(fuiRenderNode* renderNode);

    void handleCheckboxToggled(FJ_Event* event);
    static void _handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
