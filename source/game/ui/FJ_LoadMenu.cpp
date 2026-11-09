#include "game/ui/FJ_LoadMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_LoadMenu::FJ_LoadMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    m_customLoadMenu = false;
    m_unused = 0;
    m_iconY = 0;
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
}

void FJ_LoadMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
    }
}

void FJ_LoadMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_LoadMenu*>(node)->handleCheckboxToggled(event);
}

void FJ_LoadMenu::init(FJ_Event* event)
{
    m_root = getChildByName(std::string("MainPanel"));
    FJ_Document::init(event);
}

void FJ_LoadMenu::InitCustomLoadMenu()
{
    m_customLoadMenu = true;
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    m_listHeight = (int)mainPanel->getChildByName("MultiList")->getHeight();

    mainPanel->getChildByName("Seed")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("Seed"), true);
    mainPanel->getChildByName("GameModeToggle")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("GameModeToggle"), true);
    mainPanel->getChildByName("Difficulty")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("Difficulty"), true);
    mainPanel->getChildByName("TexturePackSelector")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("TexturePackSelector"), true);
    mainPanel->getChildByName("TexturePackSelectorBackground")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("TexturePackSelectorBackground"), true);
    mainPanel->getChildByName("CheckboxOnline")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("CheckboxOnline"), true);
    mainPanel->getChildByName("MoreOptions")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("MoreOptions"), true);
    mainPanel->getChildByName("LoadSettings")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("LoadSettings"), true);
    mainPanel->getChildByName("Icon")->setVisible(false);
    m_iconY = (int)mainPanel->getChildByName("Icon")->getY();
    mainPanel->removeChild(mainPanel->getChildByName("Icon"), true);
    mainPanel->getChildByName("TexturePackName")->setVisible(false);
    mainPanel->removeChild(mainPanel->getChildByName("TexturePackName"), true);

    mainPanel->getChildByName("MultiList")->m_iTabIndex = 1;
    SetFocus(-1);
}

void FJ_LoadMenu::RescaleCustomLoadMenu(bool comparison)
{
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    FJ_FuiNode* comparisonPic = mainPanel->getChildByName("ComparisonPic");
    FJ_FuiNode* levelIcon = mainPanel->getChildByName("LevelIcon");
    FJ_List_MultiList* multiList = mainPanel->getChildByName("MultiList")->asFJ_List_MultiList();
    FJ_HtmlText* description = mainPanel->getChildByName("TexturePackDescription")->asFJ_HtmlText();

    int listHeight = multiList->GetListHeight();
    int textY = m_iconY;
    int spareHeight = m_listHeight - multiList->GetListHeight();
    int descriptionHeight = (int)(description->GetRealHeight()
                                  + (comparisonPic->getY() - (description->getY() + description->GetRealHeight())));
    float comparisonY = comparisonPic->getY();
    float reclaimedHeight = (float)spareHeight;
    float offset = reclaimedHeight * 0.5f;
    int shift = Math::round(offset);
    int textTop = textY + Math::round(offset);
    int availableHeight = (int)(comparisonY - shift - textTop - textY);
    description->setHeight(descriptionHeight + levelIcon->getHeight() - reclaimedHeight);

    if (description->GetNumberOfLines() < 3)
    {
        reclaimedHeight = (float)((int)(comparisonPic->getY() + comparisonPic->getHeight() - levelIcon->getY())
                                  - (int)(description->GetOptimalHeight(-1) + comparisonPic->getHeight()));
        multiList->incY((float)m_listHeight - listHeight - reclaimedHeight);
        offset = reclaimedHeight * 0.5f;
    }
    else if (availableHeight < (int)description->GetOptimalHeight(-1))
    {
        spareHeight -= description->GetOptimalHeight(-1) - availableHeight;
        reclaimedHeight = (float)spareHeight;
        multiList->incY(m_listHeight - listHeight - spareHeight);
        offset = reclaimedHeight * 0.5f;
    }

    mainPanel->getChildByName("Background")->getChildAt(0)->decHeight(reclaimedHeight);
    mainPanel->getChildByName("Background")->incY(Math::round(offset));
    mainPanel->getChildByName("GameName")->incY(Math::round(offset));
    mainPanel->getChildByName("CreatedMode")->incY(Math::round(offset));
    mainPanel->getChildByName("LevelIcon")->incY(Math::round(offset));
    mainPanel->getChildByName("LevelIconSlot")->incY(Math::round(offset));
    mainPanel->getChildByName("MultiList")->incY(Math::round(offset));
    mainPanel->getChildByName("TextPanel")->getChildAt(0)->decHeight(reclaimedHeight);
    mainPanel->getChildByName("TextPanel")->incY(Math::round(offset));
    description->setY(textY + Math::round(offset));

    if (comparison)
    {
        description->setScaleY(1.0f);
        description->setHeight(mainPanel->getChildByName("TextPanel")->getChildAt(0)->getHeight() - textY * 2);
        mainPanel->getChildByName("ComparisonPic")->setVisible(false);
        mainPanel->removeChild(mainPanel->getChildByName("ComparisonPic"), true);
        mainPanel->getChildByName("ComparisonPicBG")->setVisible(false);
        mainPanel->removeChild(mainPanel->getChildByName("ComparisonPicBG"), true);
    }
    else
    {
        mainPanel->getChildByName("ComparisonPic")->decY(Math::round(offset));
        mainPanel->getChildByName("ComparisonPicBG")->decY(Math::round(offset));
        description->setScaleY(1.0f);
        description->setHeight(description->GetOptimalHeight(-1));
    }
}

FJ_LoadMenu* FJ_LoadMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LoadMenu(renderNode);
}

unsigned int FJ_LoadMenu::getType()
{
    return 2;
}
