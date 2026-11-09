#include "game/ui/FJ_LeaderboardEntryNormal.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LeaderboardEntryNormal::FJ_LeaderboardEntryNormal(fuiRenderNode* pRenderNode) : FJ_LeaderboardEntry(pRenderNode)
{
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
    getChildByName("StyleContainer")->setVisible(false);
}

void FJ_LeaderboardEntryNormal::HandleUpdateLabel(FJ_Event*)
{
    getChildByName("StyleContainer")->setVisible(false);
    UpdateHighlightColours();
}

void FJ_LeaderboardEntryNormal::_HandleUpdateLabel(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_LeaderboardEntryNormal*>(pNode)->HandleUpdateLabel(pEvent);
}

FJ_LeaderboardEntry* FJ_LeaderboardEntryNormal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardEntryNormal(pRenderNode);
}

unsigned int FJ_LeaderboardEntryNormal::getType()
{
    return 109;
}
