#include "game/ui/FJ_TutorialPopup.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/util/Math.hpp"

FJ_TutorialPopup::FJ_TutorialPopup(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    m_iconHolder = NULL;
    m_bitmap = NULL;
    m_backgroundPanel = NULL;
    m_titleY = (int)getChildByName(std::string("Title"))->getY();
    m_exitScreenShotY = (int)getChildByName(std::string("ExitScreenShot"))->getY();
    m_descriptionY = (int)getChildByName(std::string("Description"))->getY();
    m_iconHolderY = (int)getChildByName(std::string("IconHolder"))->getY();
    m_backgroundPanel = getChildByName(std::string("BackgroundPanel"))->getChildAt(0)->asMovieClip();
    m_titleHeight = (int)getChildByName(std::string("Title"))->getHeight();
}

FJ_TutorialPopup* FJ_TutorialPopup::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_TutorialPopup(pRenderNode);
}

unsigned int FJ_TutorialPopup::getType()
{
    return 90;
}

void FJ_TutorialPopup::SetSafeZone(int, int, int, int)
{
}

void FJ_TutorialPopup::ShowTitle(bool visible)
{
    getChildByName(std::string("Title"))->setVisible(visible);
}

void FJ_TutorialPopup::ShowExitScreenShot(bool visible)
{
    getChildByName(std::string("ExitScreenShot"))->setVisible(visible);
}

void FJ_TutorialPopup::ShowDescription(bool visible)
{
    getChildByName(std::string("Description"))->setVisible(visible);
}

void FJ_TutorialPopup::ShowIconHolder(bool visible)
{
    getChildByName(std::string("IconHolder"))->setVisible(visible);
}

void FJ_TutorialPopup::AdjustLayout(bool reposition)
{
    int y = 0;
    int height = 0;
    setY(0.0f);
    if (reposition)
    {
        if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
            setY((float)Math::round((float)m_titleHeight * 1.4f));
    }

    FJ_FuiNode* title = getChildByName(std::string("Title"));
    if (title->isVisible())
    {
        y = m_titleY;
        height = (int)((float)m_titleHeight > title->getHeight() ? (float)m_titleHeight : title->getHeight());
    }

    FJ_FuiNode* exitScreenShot = getChildByName(std::string("ExitScreenShot"));
    if (exitScreenShot->isVisible())
    {
        if (y == 0)
            y = m_exitScreenShotY;
        exitScreenShot->setY((float)(y + height));
        height += (int)exitScreenShot->getHeight();
    }

    FJ_FuiNode* description = getChildByName(std::string("Description"));
    if (description->isVisible())
    {
        if (y == 0)
            y = m_descriptionY;
        description->setY((float)(y + height));
        height += description->asFJ_HtmlText()->getTextHeight();
    }

    FJ_FuiNode* iconHolder = getChildByName(std::string("IconHolder"));
    if (iconHolder->isVisible())
    {
        if (y == 0)
            y = m_iconHolderY;
        iconHolder->setY((float)(y + height));
        height += (int)iconHolder->getHeight();
    }

    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));
    m_backgroundPanel->setHeight(2.0f * ((float)y - background->getY()) + (float)height);
}

fuiRenderNode* FJ_TutorialPopup::GetBitmapData(int type)
{
    switch (type)
    {
    case 2:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_brewing", m_pRenderNode);
    case 3:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_decoration", m_pRenderNode);
    case 4:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_food", m_pRenderNode);
    case 5:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_Materials", m_pRenderNode);
    case 6:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_mechanisms", m_pRenderNode);
    case 7:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_misc", m_pRenderNode);
    case 8:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_Redstone_and_Transport", m_pRenderNode);
    case 9:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_structures", m_pRenderNode);
    case 10:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_tools", m_pRenderNode);
    case 11:
        return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_transport", m_pRenderNode);
    }
    return (fuiRenderNode*)gFui->createNodeFromSymbol("icon_armour", m_pRenderNode);
}

void FJ_TutorialPopup::SetupIconHolder(int type)
{
    FJ_FuiNode* holder = getChildByName(std::string("IconHolder"));
    FJ_FuiNode* slotControl =
        FJ_FuiNode::asMovieClip(holder->getChildAt(0))->getChildByName(std::string("SlotControl"))->asMovieClip();
    int width = Math::round(holder->getChildAt(0)->getScaleX());

    if (m_iconHolder != NULL && m_iconHolder->getParent() != NULL)
    {
        m_iconHolder->getParent()->removeChild(m_iconHolder, true);
        m_iconHolder = NULL;
    }
    if (m_bitmap != NULL)
    {
        fuiRenderNodeTimeline* parent = static_cast<fuiRenderNodeTimeline*>(m_bitmap->getParent());
        if (parent != NULL)
        {
            parent->removeChild(m_bitmap, true);
            m_bitmap = NULL;
        }
    }

    if (type == 0)
    {
        fuiRect rect;
        rect.xMin = (float)width;
        rect.yMin = rect.xMin;
        rect.xMax = rect.xMin + holder->getWidth() - (float)(width * 2);
        rect.yMax = rect.yMin + holder->getHeight() - (float)(width * 2);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect,
                                                                            "iggy_slot_IconHolder");
        renderNode->setAlpha(0.25f);
        FJ_FuiNode* iconHolder = new FJ_FuiNode(renderNode);
        m_iconHolder = iconHolder;
        renderNode->setFuiNode(iconHolder);
        holder->addChildAt(iconHolder, holder->getNumChildren());
    }
    else
    {
        m_bitmap = GetBitmapData(type);
        m_bitmap->setX((float)width);
        m_bitmap->setY((float)width);
        m_bitmap->setWidth(holder->getWidth() - (float)(width * 2));
        m_bitmap->setHeight(holder->getHeight() - (float)(width * 2));
        holder->addChildAt(m_bitmap, holder->getNumChildren());
    }
    slotControl->setVisible(false);
}
