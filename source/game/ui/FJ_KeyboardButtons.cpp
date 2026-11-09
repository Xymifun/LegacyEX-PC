#include "game/ui/FJ_KeyboardButtons.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_KeyboardButtons::FJ_KeyboardButtons(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
    m_keyboardWidth = static_cast<int>(getWidth());
    m_keyboardHeight = static_cast<int>(getHeight());
    m_remotePressButton = false;
}

void FJ_KeyboardButtons::Init(const std::wstring& label, int id)
{
    SetLabelAlignment(2);
    FJ_Button::Init(label, id);

    std::wstring wrappedLabel = label;
    int pass = 0;
    if (m_labelTooWide)
    {
        while (m_labelTooWide && pass < 5)
        {
            for (std::wstring::size_type i = 0; i < wrappedLabel.size(); ++i)
            {
                if (wrappedLabel[i] == L' ')
                {
                    wrappedLabel[i] = L'\n';
                    break;
                }
            }
            m_labelTooWide = false;
            FJ_Button::SetLabel(wrappedLabel);
            ++pass;
        }
    }
}

void FJ_KeyboardButtons::SetLabel(const std::wstring& label)
{
    FJ_Button::SetLabel(label);

    std::wstring wrappedLabel = label;
    int pass = 0;
    if (m_labelTooWide)
    {
        while (m_labelTooWide && pass < 5)
        {
            for (std::wstring::size_type i = 0; i < wrappedLabel.size(); ++i)
            {
                if (wrappedLabel[i] == L' ')
                {
                    wrappedLabel[i] = L'\n';
                    break;
                }
            }
            m_labelTooWide = false;
            FJ_Button::SetLabel(wrappedLabel);
            ++pass;
        }
    }
}

void FJ_KeyboardButtons::RemotePressButton()
{
    m_remotePressButton = true;
    ChangeState(3);
}

void FJ_KeyboardButtons::UpdateLabel()
{
    if (getCurrentState() != 2)
    {
        FJ_FuiNode* outlineMovieClip = getChildByName(std::string("KeyboardOutline"))->asMovieClip();
        outlineMovieClip->setWidth(static_cast<float>(m_keyboardWidth + m_keyboardOffset * 2) / getScaleX());
        outlineMovieClip->setHeight(static_cast<float>(m_keyboardHeight + m_keyboardOffset * 2) / getScaleY());
        outlineMovieClip->setX(static_cast<float>(-m_keyboardOffset) / getScaleX());
        outlineMovieClip->setY(static_cast<float>(-m_keyboardOffset) / getScaleY());
    }

    if (getEnabled() && getCurrentState() == 3)
    {
        FJ_FuiNode* backgroundMovieClip = getChildByName(std::string("KeyboardBackground"))->asMovieClip();
        if (backgroundMovieClip != 0)
        {
            backgroundMovieClip->setWidth(static_cast<float>(m_keyboardWidth - m_keyboardOffset * 2) / getScaleX());
            backgroundMovieClip->setHeight(static_cast<float>(m_keyboardHeight - m_keyboardOffset * 2) / getScaleY());
            backgroundMovieClip->setX(static_cast<float>(m_keyboardOffset) / getScaleX());
            backgroundMovieClip->setY(static_cast<float>(m_keyboardOffset) / getScaleY());
        }
    }

    if (!m_focused)
    {
        if (!m_remotePressButton && getCurrentState() == 3)
            ChangeState(2);
        m_remotePressButton = false;
    }
    FJ_Button::UpdateLabel();
}
