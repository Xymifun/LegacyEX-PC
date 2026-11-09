#include "game/ui/FJ_CreditsMenu.hpp"

#include <cstdio>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_CreditsMenu::FJ_CreditsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    m_blendingContainer = getChildByName(std::string("BlendingContainer"))->asMovieClip();
    m_creditsContent = m_blendingContainer->getChildByName(std::string("CreditsContent"))->asMovieClip();

    m_scrollTop = 130;
    m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 25.0f);
    if (getStage()->getStageHeight() == 408.0f)
    {
        m_scrollTop = 110;
        m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 25.0f);
    }
    if (getStage()->getStageHeight() == 720.0f)
    {
        m_scrollTop = 195;
        m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 38.0f);
    }
    else if (getStage()->getStageHeight() == 1080.0f)
    {
        m_scrollTop = 292;
        m_scrollBottom = static_cast<int>(getStage()->getStageHeight() - 56.0f);
    }

    m_scrollAmount = 1.0f;
    if (getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 540.0f)
        m_scrollAmount = 1.5f;
    if (getStage()->getStageHeight() == 408.0f)
        m_scrollAmount = 0.5f;

    m_logoLanguage = 0;
}

FJ_CreditsMenu::~FJ_CreditsMenu()
{
    removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
}

FJ_Document* FJ_CreditsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_CreditsMenu(renderNode);
}

unsigned int FJ_CreditsMenu::getType()
{
    return 0x58;
}

void FJ_CreditsMenu::RequestNextLabel()
{
    if (getRoot()->m_pCallbackScene != NULL)
        getRoot()->m_pCallbackScene->handleRequestMoreData(0, false);
}

void FJ_CreditsMenu::UpdateScrolling(FJ_Event*)
{
    if (!m_requestingLabel && m_scrollingNodes.size() < 25)
        RequestNextLabel();

    for (unsigned int i = 0; i < m_scrollingNodes.size(); ++i)
    {
        m_scrollingNodes[i]->setY(m_scrollingNodes[i]->getY()
                                  - static_cast<float>(Math::round(m_scrollAmount + m_scrollAmount)));
    }

    if (!m_scrollingNodes.empty())
    {
        if (m_scrollingNodes[0]->getY() + m_scrollingNodes[0]->getHeight() < static_cast<float>(m_scrollTop))
        {
            if (m_scrollingNodes[0]->isFJ_LabelOutline())
                m_scrollingNodes[0]->asFJ_LabelOutline()->SetLabel(L"");
            m_scrollingNodes[0]->setVisible(false);
            m_scrollingNodes.erase(m_scrollingNodes.begin());
        }
    }
    else
    {
        m_requestingLabel = false;
    }
}

void FJ_CreditsMenu::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CreditsMenu*>(node)->UpdateScrolling(event);
}

void FJ_CreditsMenu::init(FJ_Event*)
{
    m_requestingLabel = false;
    addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);

    char name[64];
    for (int i = 0; i < 25; ++i)
    {
        std::sprintf(name, "CreditsText_S_%d", i + 1);
        m_smallLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
        m_smallLabels[i]->setX(static_cast<float>(
            Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_smallLabels[i]->getWidth() * 0.5f))));
        m_smallLabels[i]->setY(0.0f);
        m_smallLabels[i]->setVisible(false);

        if (i < 12)
        {
            std::sprintf(name, "CreditsText_M_%d", i + 1);
            m_mediumLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
            m_mediumLabels[i]->setX(static_cast<float>(
                Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_mediumLabels[i]->getWidth() * 0.5f))));
            m_mediumLabels[i]->setY(0.0f);
            m_mediumLabels[i]->setVisible(false);
        }

        if (i < 20)
        {
            std::sprintf(name, "CreditsText_L_%d", i + 1);
            m_largeLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
            m_largeLabels[i]->setX(static_cast<float>(
                Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_largeLabels[i]->getWidth() * 0.5f))));
            m_largeLabels[i]->setY(0.0f);
            m_largeLabels[i]->setVisible(false);
        }

        if (i < 8)
        {
            std::sprintf(name, "CreditsText_XL_%d", i + 1);
            m_extraLargeLabels[i] = m_creditsContent->getChildByName(std::string(name))->asFJ_LabelOutline();
            m_extraLargeLabels[i]->setX(static_cast<float>(
                Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_extraLargeLabels[i]->getWidth() * 0.5f))));
            m_extraLargeLabels[i]->setY(0.0f);
            m_extraLargeLabels[i]->setVisible(false);
        }
    }

    m_logoIggy = m_creditsContent->getChildByName(std::string("Logo_Iggy"));
    m_logoIggy->setX(
        static_cast<float>(Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_logoIggy->getWidth() * 0.5f))));
    m_logoIggy->setY(0.0f);

    m_logoMiles = m_creditsContent->getChildByName(std::string("Logo_Miles"));
    m_logoMiles->setX(
        static_cast<float>(Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_logoMiles->getWidth() * 0.5f))));
    m_logoMiles->setY(0.0f);

    m_logoDolby = m_creditsContent->getChildByName(std::string("Logo_Dolby"));
    m_logoDolby->setX(
        static_cast<float>(Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_logoDolby->getWidth() * 0.5f))));
    m_logoDolby->setY(0.0f);

    m_logoNintendo = m_creditsContent->getChildByName(std::string("Logo_Nintendo"));
    m_logoNintendo->setX(
        static_cast<float>(Math::round(getRect(this).xMin + (getWidth() * 0.5f - m_logoNintendo->getWidth() * 0.5f))));
    m_logoNintendo->setY(0.0f);

    getChildByName(std::string("LogoObject"))->getChildAt(0)->gotoAndStop(1);
}

void FJ_CreditsMenu::AddImage(int image, bool requestNext)
{
    m_requestingLabel = requestNext;

    if (image == 0)
    {
        m_logoIggy->setVisible(true);
        m_scrollingNodes.push_back(m_logoIggy);
    }
    else if (image == 1)
    {
        m_logoMiles->setVisible(true);
        m_scrollingNodes.push_back(m_logoMiles);
    }
    else if (image == 2)
    {
        m_logoDolby->setVisible(true);
        m_scrollingNodes.push_back(m_logoDolby);
    }
    else if (image == 3)
    {
        m_logoNintendo->setVisible(true);
        m_scrollingNodes.push_back(m_logoNintendo);
    }

    if (!m_scrollingNodes.empty())
    {
        if (m_scrollingNodes.size() == 1)
        {
            m_scrollingNodes[m_scrollingNodes.size() - 1]->setY(static_cast<float>(m_scrollBottom));
        }
        else
        {
            FJ_FuiNode* node = m_scrollingNodes[m_scrollingNodes.size() - 1];
            node->setY(m_scrollingNodes[m_scrollingNodes.size() - 2]->getY()
                       + m_scrollingNodes[m_scrollingNodes.size() - 2]->getHeight());
        }
    }
}

void FJ_CreditsMenu::SetLogoLanguage(int language)
{
    m_logoLanguage = language;
}

unsigned int FJ_CreditsMenu::SetNextLabel(std::wstring label, int type, bool requestNext)
{
    bool added = false;
    m_requestingLabel = requestNext;
    if (label.size() == 0)
        label = L" ";

    switch (type)
    {
    case 3:
        for (int i = 0; i < 25; ++i)
        {
            if (!m_smallLabels[i]->isVisible())
            {
                m_smallLabels[i]->setVisible(true);
                m_smallLabels[i]->SetLabel(label);
                m_scrollingNodes.push_back(m_smallLabels[i]);
                added = true;
                break;
            }
        }
        break;
    case 2:
        for (int i = 0; i < 12; ++i)
        {
            if (!m_mediumLabels[i]->isVisible())
            {
                m_mediumLabels[i]->setVisible(true);
                m_mediumLabels[i]->SetLabel(label);
                m_scrollingNodes.push_back(m_mediumLabels[i]);
                added = true;
                break;
            }
        }
        break;
    case 1:
        for (int i = 0; i < 20; ++i)
        {
            if (!m_largeLabels[i]->isVisible())
            {
                m_largeLabels[i]->setVisible(true);
                m_largeLabels[i]->SetLabel(label);
                m_scrollingNodes.push_back(m_largeLabels[i]);
                added = true;
                break;
            }
        }
        break;
    case 0:
        for (int i = 0; i < 8; ++i)
        {
            if (!m_extraLargeLabels[i]->isVisible())
            {
                m_extraLargeLabels[i]->setVisible(true);
                m_extraLargeLabels[i]->SetLabel(label);
                m_scrollingNodes.push_back(m_extraLargeLabels[i]);
                added = true;
                break;
            }
        }
        break;
    }

    if (added)
    {
        if (!m_scrollingNodes.empty())
        {
            if (m_scrollingNodes.size() == 1)
            {
                m_scrollingNodes[m_scrollingNodes.size() - 1]->setY(static_cast<float>(m_scrollBottom));
            }
            else
            {
                FJ_FuiNode* node = m_scrollingNodes[m_scrollingNodes.size() - 1];
                node->setY(m_scrollingNodes[m_scrollingNodes.size() - 2]->getY()
                           + m_scrollingNodes[m_scrollingNodes.size() - 2]->getHeight());
            }
        }
    }
    return added;
}
