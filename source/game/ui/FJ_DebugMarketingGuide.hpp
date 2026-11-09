#ifndef GAME_UI_FJ_DEBUGMARKETINGGUIDE_HPP
#define GAME_UI_FJ_DEBUGMARKETINGGUIDE_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_DebugMarketingGuide : public FJ_Document
{
public:
    FJ_DebugMarketingGuide(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetPlatform(int platform);
};

#endif
