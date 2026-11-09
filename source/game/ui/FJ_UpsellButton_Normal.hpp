#ifndef GAME_UI_FJ_UPSELLBUTTON_NORMAL_HPP
#define GAME_UI_FJ_UPSELLBUTTON_NORMAL_HPP

#include "game/ui/FJ_UpsellButton.hpp"

class fuiRenderNode;

class FJ_UpsellButton_Normal : public FJ_UpsellButton
{
public:
    explicit FJ_UpsellButton_Normal(fuiRenderNode* renderNode);
    static FJ_UpsellButton* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
