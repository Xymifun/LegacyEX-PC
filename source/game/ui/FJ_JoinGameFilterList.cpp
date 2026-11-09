#include "game/ui/FJ_JoinGameFilterList.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_JoinGameFilterList::FJ_JoinGameFilterList(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_JoinGameFilterList::Create(fuiRenderNode* renderNode)
{
    return new FJ_JoinGameFilterList(renderNode);
}

unsigned int FJ_JoinGameFilterList::getType()
{
    return 0x3f;
}

void FJ_JoinGameFilterList::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(button->FJ_Base::getObjListParent())->getId(),
                                      button->getId());
        button->setButtonPressed(false);
    }
}
