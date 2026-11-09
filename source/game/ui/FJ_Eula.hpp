#ifndef GAME_UI_FJ_EULA_HPP
#define GAME_UI_FJ_EULA_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_Eula : public FJ_Document
{
public:
    FJ_Eula(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
