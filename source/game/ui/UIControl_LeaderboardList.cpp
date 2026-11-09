#include "game/ui/UIControl_LeaderboardList.hpp"
#include "game/core/ConsoleUIController.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_LeaderboardList::UIControl_LeaderboardList() : UIControl_Base()
{
}

void UIControl_LeaderboardList::clearList()
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->ResetLeaderboard();
}

bool UIControl_LeaderboardList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_LeaderboardList;
    bool result = UIControl_Base::setupControl(scene, node, name);
    m_touchBoxAdded = false;
    return result;
}

void UIControl_LeaderboardList::init(int controlId)
{
    m_controlId = controlId;
}

void UIControl_LeaderboardList::ReInit()
{
    UIControl::ReInit();
    init(m_controlId);
}

void UIControl_LeaderboardList::setupTitles(const std::wstring& title, const std::wstring& subtitle)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetupTitles(title, subtitle);
}

void UIControl_LeaderboardList::initLeaderboard(int focusRank, int totalEntries, int numColumns)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->InitLeaderboard(focusRank, totalEntries, numColumns);
    if (!m_touchBoxAdded)
    {
        switch (m_scene->m_layer->m_layerId)
        {
        case EUILayer_Fullscreen:
        case EUILayer_Scene:
            gConsoleUIController.TouchBoxAdd(this, m_scene);
            break;
        }
        m_touchBoxAdded = true;
    }
}

void UIControl_LeaderboardList::setColumnIcon(int column, int icon)
{
    int offsetIcon = 0;
    if (icon > 32000)
        offsetIcon = icon - 32000;
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetColumnIcon(column, offsetIcon);
}

void UIControl_LeaderboardList::addDataSet(bool loading, int rank, int value, const std::wstring& gamertag,
                                           bool complete, const std::wstring& firstColumnText,
                                           const std::wstring& secondColumnText, const std::wstring& thirdColumnText,
                                           const std::wstring& fourthColumnText, const std::wstring& fifthColumnText,
                                           const std::wstring& sixthColumnText, const std::wstring& seventhColumnText)
{
    const std::wstring* columns[7];
    for (int i = 0; i < 7; ++i)
        columns[i] = NULL;
    if (!firstColumnText.empty())
        columns[0] = &firstColumnText;
    if (!secondColumnText.empty())
        columns[1] = &secondColumnText;
    if (!thirdColumnText.empty())
        columns[2] = &thirdColumnText;
    if (!fourthColumnText.empty())
        columns[3] = &fourthColumnText;
    if (!fifthColumnText.empty())
        columns[4] = &fifthColumnText;
    if (!sixthColumnText.empty())
        columns[5] = &sixthColumnText;
    if (!seventhColumnText.empty())
        columns[6] = &seventhColumnText;
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->AddDataSet(loading, rank, value, gamertag, complete,
                                                                      columns[0], columns[1], columns[2], columns[3],
                                                                      columns[4], columns[5], columns[6]);
}

void UIControl_LeaderboardList::SetTouchFocus(int x, int y, bool focus)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetTouchFocus(x, y, focus);
}
