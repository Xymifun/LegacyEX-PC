#ifndef GAME_UI_FJ_LABEL_BIGWHITE_HPP
#define GAME_UI_FJ_LABEL_BIGWHITE_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_BigWhite : public FJ_Label
{
public:
    FJ_Label_BigWhite(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}


    static FJ_Label* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
