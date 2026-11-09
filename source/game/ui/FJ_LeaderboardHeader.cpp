#include "game/ui/FJ_LeaderboardHeader.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"
#include <cstdio>

FJ_LeaderboardHeader::FJ_LeaderboardHeader(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode),
      m_rankTextWidth(0.0f),
      m_gamertagTextWidth(0.0f),
      m_small(false),
      m_slotWidth(0),
      m_slotAreaWidth(0)
{
    for (int i = 0; i < 7; ++i)
        m_icons[i] = NULL;
}

FJ_LeaderboardHeader* FJ_LeaderboardHeader::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardHeader(pRenderNode);
}

void FJ_LeaderboardHeader::Init(int count, FJ_FuiNode* parent, int x, int y)
{
    char name[64];
    for (int i = 0; i < 7; ++i)
    {
        std::sprintf(name, "Icon_%d", i);
        m_icons[i] = getChildByName(std::string(name))->asMovieClip();
        parent->addChild(m_icons[i]);
        if (m_slotWidth == 0)
            m_slotWidth = static_cast<int>(m_icons[0]->getWidth());
        m_icons[i]->setScaleX(1.0f);
        m_icons[i]->setScaleY(1.0f);
        m_icons[i]->gotoAndStop(std::string("SlotNormal"));
    }

    int iconY = static_cast<int>(m_icons[0]->getY());
    m_slotAreaWidth = static_cast<int>(m_icons[0]->getWidth());
    int left = static_cast<int>(m_icons[0]->getX());
    int right = static_cast<int>(m_icons[6]->getX());
    int spacing = (right - left) / 6;
    int slotWidth = ((right - left) - (count - 1) * spacing) / count;
    for (int i = 0; i < 7; ++i)
    {
        if (i < count)
        {
            m_entryPositions[i] = Math::round(static_cast<float>(left + i * (spacing + slotWidth))
                                              + static_cast<float>(slotWidth) * 0.5f);
            m_icons[i]->setX(
                static_cast<float>(x + Math::round(static_cast<float>(m_entryPositions[i]) * getScaleX())));
            m_icons[i]->setY(static_cast<float>(iconY + y));
            m_icons[i]->setVisible(true);
        }
        else
        {
            m_icons[i]->setVisible(false);
        }
    }
}

void FJ_LeaderboardHeader::SetupHeaders(std::wstring rank, std::wstring gamertag)
{
    FJ_FuiNode* rankContainer = getChildByName("RankContainer")->getChildAt(0)->asMovieClip();
    FJ_FuiNode* gamertagContainer = getChildByName("GamertagContainer")->getChildAt(0)->asMovieClip();
    fuiRenderNodeEditText* rankText = rankContainer->getRenderNodeChildAt(0)->asEditText();
    if (m_rankTextWidth == 0.0f)
        m_rankTextWidth = rankText->getWidth();
    rankText->setAutoSize(TextFieldAutoSizeLeft);
    rankText->setText(rank);
    rankText->setScaleX(1.0f / getScaleX());
    rankText->setScaleY(1.0f / getScaleY());
    rankText->setX(-rankText->getWidth() * 0.5f + m_rankTextWidth * 0.5f);
    fuiRenderNodeEditText* gamertagText = gamertagContainer->getRenderNodeChildAt(0)->asEditText();
    if (m_gamertagTextWidth == 0.0f)
        m_gamertagTextWidth = gamertagText->getWidth();
    gamertagText->setAutoSize(TextFieldAutoSizeLeft);
    gamertagText->setText(gamertag);
    gamertagText->setScaleX(1.0f / getScaleX());
    gamertagText->setScaleY(1.0f / getScaleY());
    gamertagText->setX(-gamertagText->getWidth() * 0.5f + m_gamertagTextWidth * 0.5f);
}

fuiRenderNode* FJ_LeaderboardHeader::GetBitmapData(int type)
{
    switch (type)
    {
    default:
        if (!m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Climbed", m_pRenderNode);
        break;
    case 2:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Fallen_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Fallen", m_pRenderNode);
    case 3:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Walked_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Walked", m_pRenderNode);
    case 4:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Swam_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Swam", m_pRenderNode);
    case 5:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Zombie_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Zombie", m_pRenderNode);
    case 6:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_ZombiePigman_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_ZombiePigman", m_pRenderNode);
    case 7:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Ghast_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Ghast", m_pRenderNode);
    case 8:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Creeper_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Creeper", m_pRenderNode);
    case 9:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Skeleton_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Skeleton", m_pRenderNode);
    case 10:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Spider_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Spider", m_pRenderNode);
    case 11:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_SpiderJockey_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_SpiderJockey", m_pRenderNode);
    case 12:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Slime_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Slime", m_pRenderNode);
    case 13:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Portal_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Portal", m_pRenderNode);
    case 14:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Score_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Score", m_pRenderNode);
    case 15:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_Time_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_Time", m_pRenderNode);
    case 16:
        if (m_small)
            return gFui->createNodeFromSymbol("LeaderBoard_Icon_TotalRings_Small", m_pRenderNode);
        return gFui->createNodeFromSymbol("LeaderBoard_Icon_TotalRings", m_pRenderNode);
    }

    return gFui->createNodeFromSymbol("LeaderBoard_Icon_Climbed_Small", m_pRenderNode);
}

void FJ_LeaderboardHeader::SetupSlot(int slot, int type)
{
    FJ_FuiNode* icon = m_icons[slot];
    FJ_FuiNode* control = m_icons[slot]->getChildByName(std::string("SlotControl"))->asMovieClip();
    m_icons[slot]->setWidth(static_cast<float>(m_slotWidth));
    m_icons[slot]->setHeight(static_cast<float>(m_slotWidth));
    if (type == 0)
    {
        fuiRect rect;
        rect.xMin = control->getX();
        rect.yMin = control->getY();
        rect.xMax = rect.xMin + control->getWidth();
        rect.yMax = rect.yMin + control->getHeight();
        char name[64];
        std::sprintf(name, "iggy_slot_%d", slot);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, name);
        renderNode->setAlpha(0.25f);
        renderNode->setFuiNode(new FJ_FuiNode(renderNode));
        m_icons[slot]->addChild(renderNode->getFuiNode());
        m_icons[slot]->removeChild(control, true);
        return;
    }

    fuiRenderNode* bitmap = GetBitmapData(type);
    bitmap->setWidth(control->getWidth());
    bitmap->setHeight(control->getHeight());
    bitmap->setX(control->getX());
    bitmap->setY(control->getY());
    m_icons[slot]->addChild(bitmap);
    m_icons[slot]->removeChild(control, true);
}

unsigned int FJ_LeaderboardHeader::getFirstEntryPos()
{
    return m_entryPositions[0];
}

int FJ_LeaderboardHeader::getEntrySpacing()
{
    return m_entryPositions[1] - m_entryPositions[0];
}

unsigned int FJ_LeaderboardHeader::getIconHolderHalfWidth()
{
    return Math::round(m_icons[0]->getWidth() * 0.5f / getScaleX());
}

void FJ_LeaderboardHeader::Destroy()
{
    int i;
    for (i = 0; i < 7; ++i)
    {
        if (m_icons[i] != NULL && m_icons[i]->getParent() != NULL)
            m_icons[i]->getParent()->removeChild(m_icons[i], true);
        m_icons[i] = NULL;
    }
}

unsigned int FJ_LeaderboardHeader::getType()
{
    return 0x69;
}

void FJ_LeaderboardHeader::ASConstructor()
{
    FJ_FuiNode* node = getChildByName("SlotArea_Small");
    if (node != NULL)
        m_small = true;
    else
        node = getChildByName("SlotArea")->asMovieClip();
    node->setVisible(false);
}
