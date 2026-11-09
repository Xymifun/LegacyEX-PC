#ifndef GAME_UI_FJ_LISTBUTTONCONTROLLER_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONCONTROLLER_SMALL_HPP

#include "game/ui/FJ_ListButtonController.hpp"

class fuiRenderNode;

class FJ_ListButtonController_Small : public FJ_ListButtonController
{
public:
    explicit FJ_ListButtonController_Small(fuiRenderNode* renderNode);
    static FJ_ListButtonController* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
