#include "game/ui/FJ_ToolTip.hpp"

#include <cstdio>
#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/util/Math.hpp"

static std::string g_toolTipLabels[16][2] = {{"Button_A", "Button_A_Animated"},
                                             {"Button_B", "Button_B_Animated"},
                                             {"Button_X", "Button_X_Animated"},
                                             {"Button_Y", "Button_Y_Animated"},
                                             {"Button_LeftTrigger", "Button_LeftTrigger_Animated"},
                                             {"Button_RightTrigger", "Button_RightTrigger_Animated"},
                                             {"Button_LeftBumper", "Button_LeftBumper_Animated"},
                                             {"Button_RightBumper", "Button_RightBumper_Animated"},
                                             {"Button_LStick", "Button_LStick_Animated"},
                                             {"Button_RStick", "Button_RStick_Animated"},
                                             {"Button_SystemSelect", "Button_SystemSelect_Animated"},
                                             {"Button_DpadLeft", "Button_Select_Animated"},
                                             {"Button_DpadRight", "Button_Select_Animated"},
                                             {"Button_DpadUp", "Button_Select_Animated"},
                                             {"Button_DpadDown", "Button_Select_Animated"},
                                             {"Button_LeftStick", "Button_LeftStick_Animated"}};
static std::string g_toolTipControllerSuffix("_CTRL_");

FJ_ToolTip::FJ_ToolTip(fuiRenderNode* renderNode)
    : FJ_Base(renderNode),
      m_controllerType(0),
      m_primaryType(-1),
      m_secondaryType(-1)
{
    m_primaryText = getChildByName(std::string("ToolTip_0"))->getChildAt(0)->asMovieClip();
    m_secondaryText = getChildByName(std::string("ToolTip_1"))->getChildAt(0)->asMovieClip();
    m_textContainer = getChildByName(std::string("FJ_TextContainer"))->asMovieClip();
    m_secondaryPanel = getChildByName(std::string("FJ_TextContainer_2"))->asMovieClip();
    m_textContainerX = static_cast<int>(m_textContainer->getX());
}

FJ_ToolTip::~FJ_ToolTip()
{
}

fuiRenderNodeEditText* FJ_ToolTip::GetSecondaryTextField()
{
    fuiRenderNodeEditText* textField = 0;
    FJ_FuiNode* node;
    if (isFJ_Label())
    {
        node = getChildAt(0);
        node = node->asMovieClip();
    }
    else
    {
        node = getChildByName(std::string("FJ_TextContainer_2"));
        node = (node != 0 && node->getNumChildren() != 0) ? node->getChildAt(0) : 0;
    }
    fuiRenderNode* timelineNode;
    if (node != 0 && (timelineNode = node->m_pRenderNode->asTimeline()) != 0)
        textField = static_cast<fuiRenderNodeTimeline*>(timelineNode)->getChildAt(0)->asEditText();
    return textField;
}

void FJ_ToolTip::SetSecondaryText(const std::wstring& text)
{
    FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer_2"))->asMovieClip();
    fuiRenderNodeEditText* field = GetSecondaryTextField();
    field->setText(text);

    if (container != NULL)
    {
        container->setScaleX(1.0f);
        container->setScaleY(1.0f);
    }
    field->setScaleX(1.0f / getScaleX());
    field->setScaleY(1.0f / getScaleY());
    field->setAutoSize(TextFieldAutoSizeLeft);
    if (container != NULL)
        container->setY(0.0f);

    field->setX(static_cast<float>(
        Math::round((container->getWidth() / getScaleX() - field->getWidth()) * 0.5f + m_centerBiasX)));
    field->setY(static_cast<float>(Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f)));
}

void FJ_ToolTip::SetType(int type, int subtype)
{
    char label[64];
    if (m_controllerType > 0)
        std::sprintf(label, "%s%d", g_toolTipControllerSuffix.c_str(), m_controllerType);
    else
        label[0] = '\0';

    if (type >= 0)
    {
        if (m_primaryType != type)
            m_primaryText->gotoAndStop(g_toolTipLabels[type][0] + label);
        m_primaryText->setVisible(true);
    }
    else
    {
        m_primaryText->setVisible(false);
    }

    if (subtype >= 0)
    {
        if (static_cast<int>(m_primaryText->getWidth()) > 0)
        {
            int width = static_cast<int>(m_primaryText->getWidth());
            m_secondaryPanel->setX(m_primaryText->getX() + static_cast<float>(width) + 1.0f);
            m_secondaryPanel->setVisible(true);
        }
        else
        {
            int width = m_textContainerX;
            m_secondaryPanel->setX(m_primaryText->getX() + static_cast<float>(width) + 1.0f);
            m_secondaryPanel->setVisible(true);
        }

        SetSecondaryText(std::wstring(L"+"));

        if (m_secondaryType != subtype)
            m_secondaryText->gotoAndStop(g_toolTipLabels[subtype][0] + label);

        m_secondaryText->setX(m_secondaryPanel->getX() + m_secondaryPanel->getWidth() + 1.0f);
        m_secondaryText->setVisible(true);
    }
    else
    {
        m_secondaryPanel->setVisible(false);
        SetSecondaryText(std::wstring(L""));
        m_secondaryText->setVisible(false);
    }

    m_primaryType = type;
    m_secondaryType = subtype;
}

void FJ_ToolTip::Init(const std::wstring& label, int id, bool visible, int type, int subtype)
{
    InitBase(label, id);
    m_controllerType = 0;
    SetType(type, subtype);
    setVisible(false);
}

void FJ_ToolTip::UpdateControllerType(int controllerType)
{
#if defined(RECON_NATIVE)
    const int primaryType = m_primaryType;
    const int secondaryType = m_secondaryType;
    if (m_controllerType != controllerType)
    {
        m_primaryType = -1;
        m_secondaryType = -1;
    }
    m_controllerType = controllerType;
    SetType(primaryType, secondaryType);
#else
    m_controllerType = controllerType;
    SetType(m_primaryType, m_secondaryType);
#endif
}

uint8_t FJ_ToolTip::IsSingleButtonToolTip()
{
    return static_cast<uint8_t>(m_secondaryText->isVisible() ^ 1);
}

void FJ_ToolTip::Animate()
{
    char label[64];
    if (m_controllerType > 0)
        std::sprintf(label, "%s%d", g_toolTipControllerSuffix.c_str(), m_controllerType);
    else
        label[0] = '\0';

    if (m_primaryText->isVisible())
        m_primaryText->gotoAndPlay(g_toolTipLabels[m_primaryType][1] + label);

    if (m_secondaryText->isVisible())
        m_secondaryText->gotoAndPlay(g_toolTipLabels[m_secondaryType][1] + label);
}

void FJ_ToolTip::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);

    if (m_primaryText->isVisible() && !m_secondaryText->isVisible())
    {
        int offset = m_textContainerX + 1;
        if (m_secondaryText->getWidth() > 0.0f)
            offset = static_cast<int>(m_secondaryText->getWidth()) + 1;

        m_textContainer->setX(m_primaryText->getRect(m_primaryText).xMin + m_primaryText->getX()
                              + static_cast<float>(offset));
        return;
    }

    if (m_primaryText->isVisible() && m_secondaryText->isVisible())
    {
        int offset = m_textContainerX + 1;
        if (m_secondaryText->getWidth() > 0.0f)
            offset = static_cast<int>(m_secondaryText->getWidth()) + 1;

        m_textContainer->setX(m_secondaryText->getRect(m_secondaryText).xMin + m_secondaryText->getX()
                              + static_cast<float>(offset));
        return;
    }

    m_textContainer->setX(0.0f);
}
