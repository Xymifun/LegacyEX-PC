#include "game/ui/FJ_DebugCreateSchematic.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DebugCreateSchematic::FJ_DebugCreateSchematic(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
    static_cast<FJ_TextInput*>(getChildByName(std::string("Name")))->m_bUseScale = true;
}

void FJ_DebugCreateSchematic::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
}

void FJ_DebugCreateSchematic::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_DebugCreateSchematic*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_DebugCreateSchematic::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugCreateSchematic(renderNode);
}

unsigned int FJ_DebugCreateSchematic::getType()
{
    return 0x34;
}
