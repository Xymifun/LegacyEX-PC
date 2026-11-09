#ifndef GAME_UI_FJ_DEBUGOPTIONS_HPP
#define GAME_UI_FJ_DEBUGOPTIONS_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_DebugOptions : public FJ_Document
{
public:
    FJ_DebugOptions(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
