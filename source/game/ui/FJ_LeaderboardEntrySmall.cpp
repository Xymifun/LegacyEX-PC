#include "game/ui/FJ_LeaderboardEntrySmall.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LeaderboardEntrySmall::FJ_LeaderboardEntrySmall(fuiRenderNode* pRenderNode) : FJ_LeaderboardEntry(pRenderNode)
{
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
    getChildByName("StyleContainer")->setVisible(false);
}

void FJ_LeaderboardEntrySmall::HandleUpdateLabel(FJ_Event*)
{
    getChildByName("StyleContainer")->setVisible(false);
    UpdateHighlightColours();
}

void FJ_LeaderboardEntrySmall::_HandleUpdateLabel(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_LeaderboardEntrySmall*>(pNode)->HandleUpdateLabel(pEvent);
}

FJ_LeaderboardEntry* FJ_LeaderboardEntrySmall::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardEntrySmall(pRenderNode);
}

unsigned int FJ_LeaderboardEntrySmall::getType()
{
    return 110;
}
