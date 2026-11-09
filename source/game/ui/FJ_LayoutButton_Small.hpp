#ifndef GAME_UI_FJ_LAYOUTBUTTON_SMALL_HPP
#define GAME_UI_FJ_LAYOUTBUTTON_SMALL_HPP

#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_LayoutButton_Small : public FJ_Button
{
public:
    explicit FJ_LayoutButton_Small(fuiRenderNode* renderNode);

    static FJ_Button* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void Init(const std::wstring& name, int id);
};

#endif
