#ifndef GAME_UI_FJ_LEADERBOARDMENU_HPP
#define GAME_UI_FJ_LEADERBOARDMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_LeaderboardMenu : public FJ_Document
{
public:
    FJ_LeaderboardMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void ASConstructor();
    void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    virtual void keyDownHandler(FJ_Event* event);
    void SetPlatformButtons(int platform);
};

#endif
