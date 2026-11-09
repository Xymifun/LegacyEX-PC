#include "game/ui/FJ_UpsellButton.hpp"

#include "game/ui/FJ_Base.hpp"

FJ_UpsellButton::FJ_UpsellButton(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
    m_textOnly = false;
    m_textContainerChanged = true;
    m_manualWrap = true;
    m_autoGrowHeight = true;
}

void FJ_UpsellButton::Init(const std::wstring& name, int id)
{
    FJ_Base::SetLabelAlignment(2);
    FJ_Button::Init(name, id);
}
