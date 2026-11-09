#ifndef GAME_UI_FJ_LISTBUTTONCONTROLLER_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONCONTROLLER_NORMAL_HPP

#include <stdint.h>
#include "game/ui/FJ_ListButtonController.hpp"

class fuiRenderNode;

class FJ_ListButtonController_Normal : public FJ_ListButtonController
{
public:
    explicit FJ_ListButtonController_Normal(fuiRenderNode* renderNode);
    static FJ_ListButtonController* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
