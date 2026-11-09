#ifndef GAME_UI_FJ_ABSTRACTCONTAINERDOCUMENT_HPP
#define GAME_UI_FJ_ABSTRACTCONTAINERDOCUMENT_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_AbstractContainerDocument : public FJ_Document
{
public:
    explicit FJ_AbstractContainerDocument(fuiRenderNode* renderNode);
    virtual ~FJ_AbstractContainerDocument() {}
    void handleMouseMove(FJ_Event* event);
    static void _handleMouseMove(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    virtual void HideInventory();
};

#endif
