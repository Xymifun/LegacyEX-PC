#ifndef GAME_UI_FJ_MENUBUTTON_HPP
#define GAME_UI_FJ_MENUBUTTON_HPP

#include <string>

#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_MenuButton : public FJ_Button
{
public:
    explicit FJ_MenuButton(fuiRenderNode* renderNode);
    virtual ~FJ_MenuButton() {}

    virtual void Init(const std::wstring& name, int id);
    void UseEllipsis(bool useEllipsis);
};

#endif
