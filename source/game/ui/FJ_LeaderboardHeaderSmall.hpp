#ifndef GAME_UI_FJ_LEADERBOARDHEADERSMALL_HPP
#define GAME_UI_FJ_LEADERBOARDHEADERSMALL_HPP

#include "game/ui/FJ_LeaderboardHeader.hpp"

class FJ_LeaderboardHeaderSmall : public FJ_LeaderboardHeader
{
public:
    FJ_LeaderboardHeaderSmall(fuiRenderNode* pRenderNode);
    static FJ_LeaderboardHeader* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
