#ifndef GAME_UI_FJ_LABEL_HUD_BIGWHITE_HPP
#define GAME_UI_FJ_LABEL_HUD_BIGWHITE_HPP

#include <string>

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_HUD_BigWhite : public FJ_Label
{
public:
    explicit FJ_Label_HUD_BigWhite(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}


    static FJ_Label* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
    virtual void SetLabel(const std::wstring& label);
};

#endif
