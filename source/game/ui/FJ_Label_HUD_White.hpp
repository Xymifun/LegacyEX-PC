#ifndef GAME_UI_FJ_LABEL_HUD_WHITE_HPP
#define GAME_UI_FJ_LABEL_HUD_WHITE_HPP

#include "game/ui/FJ_Label.hpp"

#include <string>

class fuiRenderNode;

class FJ_Label_HUD_White : public FJ_Label
{
public:
    explicit FJ_Label_HUD_White(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}

    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetLabel(const std::wstring& label);
};

#endif
