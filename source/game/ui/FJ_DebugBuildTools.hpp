#ifndef GAME_UI_FJ_DEBUGBUILDTOOLS_HPP
#define GAME_UI_FJ_DEBUGBUILDTOOLS_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_DebugBuildTools : public FJ_Document
{
public:
    FJ_DebugBuildTools(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
