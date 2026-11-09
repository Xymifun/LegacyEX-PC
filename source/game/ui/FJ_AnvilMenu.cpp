#include "game/ui/FJ_AnvilMenu.hpp"

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_AnvilMenu::FJ_AnvilMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
    SetCostLabel(L"", false);
    ShowRedCross(false);
}

void FJ_AnvilMenu::SetCostLabel(const std::wstring& label, bool safe)
{
    FJ_Base* green = getChildByName("MainPanel.costLabelGreen")->asFJ_Base();
    FJ_Base* red = getChildByName("MainPanel.costLabelRed")->asFJ_Base();
    if (label.empty())
    {
        green->setVisible(false);
        red->setVisible(false);
        return;
    }
    if (safe)
        green->SetLabel(label);
    else
        red->SetLabel(label);
    green->setVisible(safe);
    red->setVisible(!safe);
}

void FJ_AnvilMenu::ShowRedCross(bool show)
{
    getChildByName("MainPanel.AnvilCross")->setVisible(show);
}

FJ_AnvilMenu* FJ_AnvilMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_AnvilMenu(renderNode);
}

unsigned int FJ_AnvilMenu::getType()
{
    return 0x1b;
}

void FJ_AnvilMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;
    setX(-right);
    setY(top);
}
