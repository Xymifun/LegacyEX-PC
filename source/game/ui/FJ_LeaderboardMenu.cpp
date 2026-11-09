#include "game/ui/FJ_LeaderboardMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LeaderboardMenu::FJ_LeaderboardMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_LeaderboardMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LeaderboardMenu(renderNode);
}

unsigned int FJ_LeaderboardMenu::getType()
{
    return 0x19;
}

void FJ_LeaderboardMenu::ASConstructor()
{
    FJ_Label* infoLabel = getChildByName("Info")->asFJ_Label();
    infoLabel->Init(L"");

    FJ_LeaderboardList* leaderboardList = static_cast<FJ_LeaderboardList*>(getChildByName("Gamers"));
    leaderboardList->SetupPleaseWait(infoLabel);
}

void FJ_LeaderboardMenu::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Document*>(node)->keyDownHandler(event);
}

void FJ_LeaderboardMenu::keyDownHandler(FJ_Event* event)
{
    FJ_KeyboardEvent* keyEvent = static_cast<FJ_KeyboardEvent*>(event);
    FJ_FuiNode* gamersNode = getChildByName("Gamers");
    FJ_LeaderboardList* leaderboardList = static_cast<FJ_LeaderboardList*>(gamersNode);

    if (keyEvent->m_iKeyCode == 3)
    {
        leaderboardList->NavigateLeaderboard(true, false);
    }
    else if (keyEvent->m_iKeyCode == 4)
    {
        leaderboardList->NavigateLeaderboard(false, false);
    }
    else if (keyEvent->m_iKeyCode == 1)
    {
        leaderboardList->NavigateLeaderboard(true, true);
    }
    else if (keyEvent->m_iKeyCode == 2)
    {
        leaderboardList->NavigateLeaderboard(false, true);
    }
}
