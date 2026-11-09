#ifndef GAME_UI_FJ_LAYOUTBUTTON_HPP
#define GAME_UI_FJ_LAYOUTBUTTON_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_LayoutButton : public FJ_Button
{
public:
    explicit FJ_LayoutButton(fuiRenderNode* renderNode);

    static FJ_Button* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    virtual void Init(const std::wstring& name, int id);
};

#endif
