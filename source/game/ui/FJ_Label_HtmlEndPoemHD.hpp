#ifndef GAME_UI_FJ_LABEL_HTMLENDPOEMHD_HPP
#define GAME_UI_FJ_LABEL_HTMLENDPOEMHD_HPP

#include <string>

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_HtmlEndPoemHD : public FJ_Label
{
public:
    FJ_Label_HtmlEndPoemHD(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}


    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetLabel(const std::wstring& label);
};

#endif
