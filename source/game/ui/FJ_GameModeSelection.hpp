#ifndef GAME_UI_FJ_GAMEMODESELECTION_HPP
#define GAME_UI_FJ_GAMEMODESELECTION_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class fuiRenderNode;

class FJ_GameModeSelection : public FJ_Document
{
public:
    explicit FJ_GameModeSelection(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void handlePress(FJ_Event* event);
};

#endif
