#include "game/ui/FJ_Gamertag.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_Gamertag::FJ_Gamertag(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_Gamertag::Create(fuiRenderNode* renderNode)
{
    return new FJ_Gamertag(renderNode);
}

unsigned int FJ_Gamertag::getType()
{
    return 0x3e;
}

void FJ_Gamertag::SetGamertag(const std::wstring& gamertag)
{
    FJ_FuiNode* labelNode = getChildByName("LabelGamertag");
    FJ_Label* label = labelNode->asFJ_Label();
    label->SetLabel(gamertag);
    labelNode->setX(static_cast<float>(getStage()->getStageWidth() - labelNode->getWidth() - getISafeZoneRight()));
    labelNode->setY(static_cast<float>(getISafeZoneTop()));
}
