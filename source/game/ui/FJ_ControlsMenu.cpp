#include "game/ui/FJ_ControlsMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlTextPanel.hpp"
#include "game/ui/FJ_HtmlTextPanelLarge.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_ControlsMenu::FJ_ControlsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
}

void FJ_ControlsMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
}

void FJ_ControlsMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_ControlsMenu*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_ControlsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_ControlsMenu(renderNode);
}

unsigned int FJ_ControlsMenu::getType()
{
    return 0x25;
}

void FJ_ControlsMenu::init(FJ_Event* event)
{
    FJ_Document::init(event);

    FJ_FuiNode* controlsList = getChildByName(std::string("ControlsMultiList"));
    FJ_FuiNode* helpPanel = getChildByName(std::string("ControlsHelpPanel"));
    if (helpPanel->isFJ_HtmlTextPanelLarge())
    {
        FJ_HtmlTextPanelLarge* largePanel = helpPanel->asFJ_HtmlTextPanelLarge();
        largePanel->SetFixedWidth(Math::round(controlsList->getWidth() * 0.5f));
    }
    else if (helpPanel->isFJ_HtmlTextPanel())
    {
        FJ_HtmlTextPanel* panel = helpPanel->asFJ_HtmlTextPanel();
        panel->SetFixedWidth(Math::round(controlsList->getWidth() * 0.5f));
    }
}

void FJ_ControlsMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    FJ_FuiNode* version = getChildByName(std::string("Version"));
    if (version != NULL)
    {
        version->setX(getStage()->getStageWidth() - version->getWidth() - static_cast<float>(right));
        version->setY(getStage()->getStageHeight() - version->getHeight() - static_cast<float>(bottom)
                      - version->getHeight());
    }
}

void FJ_ControlsMenu::PositionControlsHelp()
{
    FJ_List* controlsList = static_cast<FJ_List*>(m_root->getChildByName(std::string("ControlsMultiList")));
    FJ_FuiNode* activeItem = controlsList->ReturnItem(controlsList->GetActiveItem());
    if (activeItem != NULL)
    {
        const float padding = 5.0f;

        getChildByName(std::string("ControlsHelpPanel"))
            ->setX(static_cast<float>(Math::round(getChildByName(std::string("BackgroundPanel"))->getX()
                                                  + getChildByName(std::string("BackgroundPanel"))->getWidth()
                                                  - padding)));

        getChildByName(std::string("ControlsHelpPanel"))
            ->setY(static_cast<float>(
                Math::round(activeItem->getY() + activeItem->getHeight() * 0.5f
                            - getChildByName(std::string("ControlsHelpPanel"))->getHeight() * 0.5f)));

        if (getChildByName(std::string("ControlsHelpPanel"))->getY()
                + getChildByName(std::string("ControlsHelpPanel"))->getHeight()
            > getChildByName(std::string("BackgroundPanel"))->getY()
                  + getChildByName(std::string("BackgroundPanel"))->getHeight())
        {
            getChildByName(std::string("ControlsHelpPanel"))
                ->setY(
                    getChildByName(std::string("ControlsHelpPanel"))->getY()
                    - static_cast<float>(Math::round(getChildByName(std::string("ControlsHelpPanel"))->getY()
                                                     + getChildByName(std::string("ControlsHelpPanel"))->getHeight()
                                                     - (getChildByName(std::string("BackgroundPanel"))->getY()
                                                        + getChildByName(std::string("BackgroundPanel"))->getHeight())))
                    - padding);
        }
        else if (getChildByName(std::string("ControlsHelpPanel"))->getY()
                 < getChildByName(std::string("BackgroundPanel"))->getY())
        {
            getChildByName(std::string("ControlsHelpPanel"))
                ->setY(getChildByName(std::string("ControlsHelpPanel"))->getY()
                       + static_cast<float>(Math::round(getChildByName(std::string("BackgroundPanel"))->getY()
                                                        - getChildByName(std::string("ControlsHelpPanel"))->getY()))
                       + padding);
        }
    }
}

void FJ_ControlsMenu::SetControlsHelp(int id, const std::wstring& help)
{
    FJ_FuiNode* helpPanel = getChildByName(std::string("ControlsHelpPanel"));
    if (helpPanel->isFJ_HtmlTextPanelLarge())
        helpPanel->asFJ_HtmlTextPanelLarge()->SetLabel(help);
    else if (helpPanel->isFJ_HtmlTextPanel())
        helpPanel->asFJ_HtmlTextPanel()->SetLabel(help);

    PositionControlsHelp();
}
