#ifndef GAME_UI_FJ_LEADERBOARDENTRYSMALL_HPP
#define GAME_UI_FJ_LEADERBOARDENTRYSMALL_HPP

#include "game/ui/FJ_LeaderboardEntry.hpp"

class FJ_LeaderboardEntrySmall : public FJ_LeaderboardEntry
{
public:
    FJ_LeaderboardEntrySmall(fuiRenderNode* pRenderNode);

    void HandleUpdateLabel(FJ_Event* pEvent);
    static void _HandleUpdateLabel(FJ_FuiNode* pNode, FJ_Event* pEvent);
    static FJ_LeaderboardEntry* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
