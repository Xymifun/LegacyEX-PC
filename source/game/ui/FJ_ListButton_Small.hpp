#ifndef GAME_UI_FJ_LISTBUTTON_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTON_SMALL_HPP

#include "game/ui/FJ_ListButton.hpp"

class fuiRenderNode;

class FJ_ListButton_Small : public FJ_ListButton
{
public:
    explicit FJ_ListButton_Small(fuiRenderNode* renderNode);
    static FJ_ListButton* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
