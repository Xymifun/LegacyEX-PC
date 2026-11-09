#ifndef GAME_UI_FJ_LABEL_HUD_SMALLWHITE_HPP
#define GAME_UI_FJ_LABEL_HUD_SMALLWHITE_HPP

#include <string>

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_HUD_SmallWhite : public FJ_Label
{
public:
    explicit FJ_Label_HUD_SmallWhite(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}


    static FJ_Label* Create(fuiRenderNode* renderNode);

    virtual unsigned int getType();
    virtual void SetLabel(const std::wstring& label);
};

#endif
