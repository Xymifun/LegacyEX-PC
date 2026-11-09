#ifndef GAME_UI_FJ_SAVEMESSAGE_HPP
#define GAME_UI_FJ_SAVEMESSAGE_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SaveMessage : public FJ_Document
{
public:
    FJ_SaveMessage(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    void AutoResize();
    virtual unsigned int getType();
};

#endif
