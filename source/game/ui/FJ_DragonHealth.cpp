#include "game/ui/FJ_DragonHealth.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_Label.hpp"

FJ_DragonHealth::FJ_DragonHealth(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    GetBarFiller();
    initFiller();
    m_pLabel = getChildByName("DragonHealthLabel")->asFJ_Label();
    m_pLabel->SetLabelAlignment(0);
    m_pLabel->SetTextAlignment(FJ_Label::eTextAlignment_Center);
}

void FJ_DragonHealth::Init(const std::wstring& label)
{
    m_pLabel->Init(label);
}

void FJ_DragonHealth::SetLabel(const std::wstring& label)
{
    m_pLabel->SetLabel(label);
}
