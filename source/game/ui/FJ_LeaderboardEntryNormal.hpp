#ifndef GAME_UI_FJ_LEADERBOARDENTRYNORMAL_HPP
#define GAME_UI_FJ_LEADERBOARDENTRYNORMAL_HPP

#include "game/ui/FJ_LeaderboardEntry.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_LeaderboardEntryNormal : public FJ_LeaderboardEntry
{
public:
    FJ_LeaderboardEntryNormal(fuiRenderNode* pRenderNode);

    void HandleUpdateLabel(FJ_Event* pEvent);
    static void _HandleUpdateLabel(FJ_FuiNode* pNode, FJ_Event* pEvent);
    static FJ_LeaderboardEntry* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
