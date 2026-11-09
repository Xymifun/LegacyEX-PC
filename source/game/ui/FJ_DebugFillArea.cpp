#include "game/ui/FJ_DebugFillArea.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DebugFillArea::FJ_DebugFillArea(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
    static_cast<FJ_TextInput*>(getChildByName(std::string("EdgeTile")))->m_bUseScale = true;
    static_cast<FJ_TextInput*>(getChildByName(std::string("EdgeData")))->m_bUseScale = true;
    static_cast<FJ_TextInput*>(getChildByName(std::string("FillTile")))->m_bUseScale = true;
    static_cast<FJ_TextInput*>(getChildByName(std::string("FillData")))->m_bUseScale = true;
}

void FJ_DebugFillArea::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
}

void FJ_DebugFillArea::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_DebugFillArea*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_DebugFillArea::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugFillArea(renderNode);
}

unsigned int FJ_DebugFillArea::getType()
{
    return 0x35;
}
