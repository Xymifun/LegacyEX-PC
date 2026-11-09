#include "game/ui/FJ_DLCMainMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"

FJ_DLCMainMenu::FJ_DLCMainMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_DLCMainMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DLCMainMenu(renderNode);
}

unsigned int FJ_DLCMainMenu::getType()
{
    return 59;
}

void FJ_DLCMainMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        m_pCallbackScene->handlePress(button->getObjListParent()->getId(), button->getId());
        button->setButtonPressed(false);
    }
}
