#ifndef GAME_UI_FJ_UPSELLBUTTON_SMALL_HPP
#define GAME_UI_FJ_UPSELLBUTTON_SMALL_HPP

#include "game/ui/FJ_UpsellButton.hpp"

class fuiRenderNode;

class FJ_UpsellButton_Small : public FJ_UpsellButton
{
public:
    explicit FJ_UpsellButton_Small(fuiRenderNode* renderNode);
    static FJ_UpsellButton* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
