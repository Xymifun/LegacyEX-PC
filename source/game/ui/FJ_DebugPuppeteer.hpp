#ifndef GAME_UI_FJ_DEBUGPUPPETEER_HPP
#define GAME_UI_FJ_DEBUGPUPPETEER_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_DebugPuppeteer : public FJ_Document
{
public:
    FJ_DebugPuppeteer(fuiRenderNode* renderNode);
    static FJ_DebugPuppeteer* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
