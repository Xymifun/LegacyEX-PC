#include "game/ui/FJ_JoinMenu.hpp"

#include <string>

#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_JoinMenu::FJ_JoinMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    getChildByName(std::string("JoinGame"))->asFJ_Button()->HideUntilInit();
}

FJ_Document* FJ_JoinMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_JoinMenu(renderNode);
}

uint32_t FJ_JoinMenu::getType()
{
    return 0x40;
}
