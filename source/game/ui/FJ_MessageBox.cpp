#include "game/ui/FJ_MessageBox.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_MessageBox::FJ_MessageBox(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    m_titleHeight = static_cast<int>(getChildByName(std::string("Title"))->getHeight());
    m_contentY = static_cast<int>(getChildByName(std::string("Content"))->getY());
    m_buttonCount = 0;
}

FJ_Document* FJ_MessageBox::Create(fuiRenderNode* renderNode)
{
    return new FJ_MessageBox(renderNode);
}

unsigned int FJ_MessageBox::getType()
{
    return 15;
}

void FJ_MessageBox::Init(int buttonCount, int focusIndex)
{
    FJ_FuiNode* button0 = getChildByName(std::string("Button0"));
    FJ_FuiNode* button1 = getChildByName(std::string("Button1"));
    FJ_FuiNode* button2 = getChildByName(std::string("Button2"));
    FJ_FuiNode* button3 = getChildByName(std::string("Button3"));

    m_buttonCount = buttonCount;
    m_focusChange = false;
    if (buttonCount > 4)
    {
        return;
    }

    if (buttonCount < 4)
    {
        button0->setVisible(false);
        if (buttonCount < 3)
        {
            button1->setVisible(false);
        }
        if (buttonCount < 2)
        {
            button2->setVisible(false);
        }
        if (buttonCount == 1)
        {
            button3->asFJ_Base()->m_navUpName = "";
            button3->asFJ_Base()->m_navDownName = "";
            getStage()->setFocus(button3);
            return;
        }
    }

    if (buttonCount == 2)
    {
        button2->asFJ_Base()->m_navUpName = button3->getName();
        button3->asFJ_Base()->m_navDownName = button2->getName();
        if (focusIndex == 0)
        {
            getStage()->setFocus(button2);
        }
        else if (focusIndex == 1)
        {
            getStage()->setFocus(button3);
        }
    }
    else if (buttonCount == 3)
    {
        button1->asFJ_Base()->m_navUpName = button3->getName();
        button3->asFJ_Base()->m_navDownName = button1->getName();
        if (focusIndex == 0)
        {
            getStage()->setFocus(button1);
        }
        else if (focusIndex == 1)
        {
            getStage()->setFocus(button2);
        }
        else if (focusIndex == 2)
        {
            getStage()->setFocus(button3);
        }
    }
    else if (buttonCount == 4)
    {
        if (focusIndex == 0)
        {
            getStage()->setFocus(button0);
        }
        else if (focusIndex == 1)
        {
            getStage()->setFocus(button1);
        }
        else if (focusIndex == 2)
        {
            getStage()->setFocus(button2);
        }
        else if (focusIndex == 3)
        {
            getStage()->setFocus(button3);
        }
    }
}

#ifdef MINECRAFT_PC
void FJ_MessageBox::Widen(float fraction)
{
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));
    FJ_Base* title = getChildByName(std::string("Title"))->asFJ_Base();
    FJ_Base* content = getChildByName(std::string("Content"))->asFJ_Base();
    const float margin = 48.0f;
    float extra = background->getWidth() * fraction;
    const float room = getStage()->getStageWidth() - background->getWidth() - margin * 2.0f;
    if (extra > room)
        extra = room;
    const int widthDelta = Math::round(extra);
    if (widthDelta <= 0)
        return;
    const float shift = static_cast<float>(Math::round(extra * 0.5f));
    background->getChildAt(0)->setWidth(static_cast<float>(Math::round(background->getWidth() + extra)));
    background->decX(shift);
    title->SetNewObjectWidth(static_cast<int>(title->getWidth()) + widthDelta);
    title->decX(shift);
    content->SetNewObjectWidth(static_cast<int>(content->getWidth()) + widthDelta);
    content->decX(shift);
}
#endif

void FJ_MessageBox::AutoResize()
{
    FJ_FuiNode* button0 = getChildByName(std::string("Button0"));
    FJ_FuiNode* button1 = getChildByName(std::string("Button1"));
    FJ_FuiNode* button2 = getChildByName(std::string("Button2"));
    FJ_FuiNode* button3 = getChildByName(std::string("Button3"));
    FJ_Base* title = getChildByName(std::string("Title"))->asFJ_Base();
    FJ_Base* content = getChildByName(std::string("Content"))->asFJ_Base();
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));

    const int oldBackgroundWidth = static_cast<int>(background->getWidth());
    DoHorizontalResizeCheck();
    const int backgroundWidth = static_cast<int>(background->getWidth());
    if (oldBackgroundWidth < backgroundWidth)
    {
        const float widthDelta = static_cast<float>(backgroundWidth - oldBackgroundWidth);
        title->SetNewObjectWidth(static_cast<int>(title->getWidth() + widthDelta));
        content->SetNewObjectWidth(static_cast<int>(content->getWidth() + widthDelta));
    }

    if (static_cast<float>(m_titleHeight) < title->getHeight())
    {
        const int extraHeight = static_cast<int>(title->getHeight() - static_cast<float>(m_titleHeight));
        if (extraHeight > 0)
        {
            content->setY(static_cast<float>(m_contentY + extraHeight));
        }
        m_titleHeight = static_cast<int>(title->getHeight());
    }

    int offset = 0;
    switch (m_buttonCount)
    {
    case 1:
        offset = Math::round((content->getY() + content->getHeight() - button3->getY()) + button3->getHeight() * 0.5f);
        button3->incY(static_cast<float>(offset));
        break;
    case 2:
        offset = Math::round((content->getY() + content->getHeight() - button2->getY()) + button2->getHeight() * 0.5f);
        button2->incY(static_cast<float>(offset));
        button3->incY(static_cast<float>(offset));
        break;
    case 3:
        offset = Math::round((content->getY() + content->getHeight() - button1->getY()) + button1->getHeight() * 0.5f);
        button1->incY(static_cast<float>(offset));
        button2->incY(static_cast<float>(offset));
        button3->incY(static_cast<float>(offset));
        break;
    case 4:
        offset = Math::round((content->getY() + content->getHeight() - button0->getY()) + button0->getHeight() * 0.5f);
        button0->incY(static_cast<float>(offset));
        button1->incY(static_cast<float>(offset));
        button2->incY(static_cast<float>(offset));
        button3->incY(static_cast<float>(offset));
        break;
    default:
        break;
    }

    background->getChildAt(0)->asMovieClip()->incHeight(static_cast<float>(offset));
    int i;
    for (i = 0; i < m_root->getNumChildren(); ++i)
    {
        getChildAt(i)->decY(static_cast<float>(Math::round(static_cast<float>(offset) * 0.5f)));
    }
}
