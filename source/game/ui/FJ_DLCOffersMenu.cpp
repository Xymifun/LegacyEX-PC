#include "game/ui/FJ_DLCOffersMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIScene.hpp"

FJ_DLCOffersMenu::FJ_DLCOffersMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_DLCOffersMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DLCOffersMenu(renderNode);
}

unsigned int FJ_DLCOffersMenu::getType()
{
    return 60;
}

void FJ_DLCOffersMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(button->FJ_Base::getObjListParent())->getId(),
                                      button->getId());
        button->setButtonPressed(false);
    }
}
