#ifndef GAME_UI_FJ_UPSELLBUTTON_HPP
#define GAME_UI_FJ_UPSELLBUTTON_HPP

#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_UpsellButton : public FJ_Button
{
public:
    explicit FJ_UpsellButton(fuiRenderNode* renderNode);
    virtual ~FJ_UpsellButton() {}
    virtual void Init(const std::wstring& name, int id);
};

#endif
