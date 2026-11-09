#include "game/ui/FJ_MultilistMenu.hpp"

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/util/Math.hpp"

FJ_MultilistMenu::FJ_MultilistMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
}

void FJ_MultilistMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(checkBox->getObjListParent())->getId(), checkBox->getId());
    }
}

void FJ_MultilistMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_MultilistMenu*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_MultilistMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_MultilistMenu(renderNode);
}

unsigned int FJ_MultilistMenu::getType()
{
    return 42;
}

void FJ_MultilistMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        setX(-right);
    }
}

void FJ_MultilistMenu::DoVerticalResizeCheck()
{
    FJ_FuiNode* multiList = getChildByName("MultiList");
    FJ_FuiNode* backgroundList = getChildByName("BackgroundPanel_List");
    FJ_FuiNode* backgroundText = getChildByName("BackgroundPanel_Text");
    FJ_FuiNode* description = getChildByName("Description");

    float listHeight = static_cast<float>(multiList->asFJ_List_MultiList()->GetListHeight());
    if (listHeight < multiList->getHeight())
    {
        int oldChildHeight = static_cast<int>(backgroundList->getChildAt(0)->getHeight());
        backgroundList->getChildAt(0)->setHeight(listHeight + 2.0f * (multiList->getY() - backgroundList->getY()));
        backgroundText->getChildAt(0)->setHeight(backgroundList->getChildAt(0)->getHeight()
                                                 - 2.0f * (backgroundText->getY() - backgroundList->getY()));

        if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        {
            int targetY = static_cast<int>(
                static_cast<float>(Math::round(static_cast<float>(oldChildHeight) / 3.0f
                                               - backgroundList->getChildAt(0)->getHeight() * 0.5f))
                + backgroundList->getY());
            int safeDelta = static_cast<int>(static_cast<float>(targetY) - backgroundList->getY());
            if (safeDelta > getISafeZoneBottom())
            {
                safeDelta -= getISafeZoneBottom();
            }
            if (backgroundList->getY() + static_cast<float>(safeDelta) > backgroundList->getY())
            {
                multiList->incY(static_cast<float>(safeDelta));
                backgroundList->incY(static_cast<float>(safeDelta));
                description->incY(static_cast<float>(safeDelta));
                backgroundText->incY(static_cast<float>(safeDelta));
                description->setHeight(backgroundText->getHeight()
                                       - 2.0f * (description->getY() - backgroundText->getY()));
            }
        }
    }

    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        float stageHeight = getStage()->getStageHeight();
        int rounded = Math::round(stageHeight * 0.5f - backgroundList->getChildAt(0)->getHeight() * 0.5f);
        int safeDelta = static_cast<int>(static_cast<float>(rounded) - backgroundList->getY());
        if (safeDelta > getISafeZoneBottom())
        {
            safeDelta -= getISafeZoneBottom();
        }
        float delta = static_cast<float>(safeDelta);
        multiList->incY(delta);
        backgroundList->incY(delta);
        description->incY(delta);
        backgroundText->incY(delta);
        description->setHeight(backgroundText->getHeight() - 2.0f * (description->getY() - backgroundText->getY()));
    }
}

void FJ_MultilistMenu::DoHorizontalResizeCheck()
{
    FJ_List_MultiList* multiList = getChildByName("MultiList")->asFJ_List_MultiList();
    FJ_FuiNode* backgroundList = getChildByName("BackgroundPanel_List");
    FJ_FuiNode* backgroundText = getChildByName("BackgroundPanel_Text");
    FJ_FuiNode* description = getChildByName("Description");

    float listWidth = static_cast<float>(multiList->GetListWidth());
    if (listWidth > multiList->getWidth())
    {
        listWidth = static_cast<float>(multiList->GetListWidth());
        int widthDelta = static_cast<int>(listWidth - multiList->getWidth());
        std::wstring savedLabel;

        backgroundList->getChildAt(0)->incWidth(static_cast<float>(widthDelta));
        multiList->incWidth(static_cast<float>(widthDelta));

        for (int itemIndex = 0; itemIndex < multiList->getINumButtons(); ++itemIndex)
        {
            savedLabel = static_cast<FJ_Base*>(multiList->ReturnItem(itemIndex))->GetLabel();
            static_cast<FJ_Base*>(multiList->ReturnItem(itemIndex))->SetLabel(std::wstring(L""));
            int newWidth = static_cast<int>(multiList->ReturnItem(itemIndex)->getWidth()
                                            + static_cast<float>(widthDelta));
            static_cast<FJ_Base*>(multiList->ReturnItem(itemIndex))->SetNewObjectWidth(newWidth);
            static_cast<FJ_Base*>(multiList->ReturnItem(itemIndex))->SetLabel(savedLabel);
        }

        float halfDelta = static_cast<float>(widthDelta) * 0.5f;
        multiList->decX(static_cast<float>(Math::round(halfDelta)));
        backgroundList->decX(static_cast<float>(Math::round(halfDelta)));
        if (description->isVisible())
        {
            description->incX(static_cast<float>(Math::round(halfDelta)));
            backgroundText->incX(static_cast<float>(Math::round(halfDelta)));
        }
    }
}

void FJ_MultilistMenu::HideDescription()
{
    FJ_List_MultiList* multiList = getChildByName("MultiList")->asFJ_List_MultiList();
    FJ_FuiNode* backgroundList = getChildByName("BackgroundPanel_List");
    FJ_FuiNode* backgroundText = getChildByName("BackgroundPanel_Text");
    FJ_FuiNode* description = getChildByName("Description");

    description->setVisible(false);
    backgroundText->setVisible(false);

    multiList->setX(static_cast<float>(Math::round(getStage()->getStageWidth() * 0.5f - multiList->getWidth() * 0.5f)));
    backgroundList->setX(
        static_cast<float>(Math::round(getStage()->getStageWidth() * 0.5f - backgroundList->getWidth() * 0.5f)));
}
