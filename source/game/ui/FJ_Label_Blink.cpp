#include "game/ui/FJ_Label_Blink.hpp"

#include <cstdlib>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

static std::string s_blinkLabels[2] = {std::string("Label_Stop"), std::string("Label_Off")};

FJ_Label_Blink::FJ_Label_Blink(fuiRenderNode* renderNode) : FJ_Base(renderNode), m_blinkEnabled(false)
{
}

void FJ_Label_Blink::UpdateLabel()
{
    SetLabel(m_label);
}

void FJ_Label_Blink::HandleUpdateLabel(FJ_Event*)
{
    UpdateLabel();
}

void FJ_Label_Blink::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Label_Blink*>(node)->HandleUpdateLabel(event);
}

void FJ_Label_Blink::Init(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(m_blinkAlignment);
    FJ_Base::InitBase(label, -1);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_Label_Blink::EnableBlink(bool enable)
{
    m_blinkEnabled = enable;
    if (enable)
    {
        gotoAndPlay(s_blinkLabels[1]);
    }
    else
    {
        gotoAndStop(s_blinkLabels[0]);
    }
}

void FJ_Label_Blink::SetLabel(const std::wstring& label)
{
    if (!m_initialised)
    {
        addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
    }
    FJ_Base::SetLabelAlignment(m_blinkAlignment);
    m_initialised = true;
    m_textContainerChanged = false;
    m_textContainerVisible = false;
    m_textOnly = false;
    m_manualWrap = false;
    FJ_Base::SetLabel(label);
}

void FJ_Label_Blink::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    int maxLabelWidth = m_maxLabelWidth;
    int width = maxLabelWidth > textField->m_textWidth ? maxLabelWidth : textField->m_textWidth;
    width = width > getWidth() ? width : getWidth();
    m_newWidth = std::max(m_newWidth, width);
}

void FJ_Label_Blink::setAction(unsigned char action, char* value)
{
    if (action == 5)
    {
        m_blinkAlignment = std::atoi(value);
    }
}
