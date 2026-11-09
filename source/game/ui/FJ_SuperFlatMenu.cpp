#include "game/ui/FJ_SuperFlatMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_SuperFlatMenu::FJ_SuperFlatMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_activeTab(0),
      m_currentTab(-1),
      m_helpTextSet(false),
      m_layersFocus(-1),
      m_biomesFocus(-1),
      m_propertiesFocus(-1)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);

    getChildByName("Layers.TabLeft")->getChildAt(0)->gotoAndStop(1);
    getChildByName("Biomes.TabMiddle")->getChildAt(0)->gotoAndStop(2);
    getChildByName("Properties.TabRight")->getChildAt(0)->gotoAndStop(3);
    SetActiveTab(m_activeTab);
}

void FJ_SuperFlatMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handlePress(static_cast<FJ_Base*>(checkBox->getObjListParent())->getId(), checkBox->getId());
}

void FJ_SuperFlatMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SuperFlatMenu*>(node)->handleCheckboxToggled(event);
}

void FJ_SuperFlatMenu::SetActiveTab(int tab)
{
    FJ_FuiNode* layers = getChildByName("Layers");
    FJ_FuiNode* layersHelp = layers->getChildByName("LayersHelpHtml");
    FJ_FuiNode* layersList = layers->getChildByName("LayersList");
    FJ_FuiNode* biomes = getChildByName("Biomes");
    FJ_FuiNode* biomesHelp = biomes->getChildByName("BiomesHelpHtml");
    FJ_FuiNode* biomesList = biomes->getChildByName("BiomesList");
    FJ_FuiNode* properties = getChildByName("Properties");
    FJ_FuiNode* propertiesHelp = properties->getChildByName("PropertiesHelpHtml");
    FJ_FuiNode* propertiesList = properties->getChildByName("PropertiesList");

    if (!m_helpTextSet)
    {
        layersHelp->asFJ_HtmlText()->Init(L"");
        biomesHelp->asFJ_HtmlText()->Init(L"");
        propertiesHelp->asFJ_HtmlText()->Init(L"");
        m_helpTextSet = true;
    }

    FJ_FuiNode* activeTab;
    int focus;
    const char* label;
    switch (tab)
    {
    case 0:
        layers->setVisible(true);
        biomes->setVisible(false);
        properties->setVisible(false);
        activeTab = layers;
        focus = m_layersFocus;
        label = "Layers";
        break;
    case 1:
        layers->setVisible(false);
        biomes->setVisible(true);
        properties->setVisible(false);
        activeTab = biomes;
        focus = m_biomesFocus;
        label = "Biomes";
        break;
    case 2:
        layers->setVisible(false);
        biomes->setVisible(false);
        properties->setVisible(true);
        activeTab = properties;
        focus = m_propertiesFocus;
        label = "Properties";
        break;
    default:
        m_activeTab = tab;
        m_currentTab = tab;
        return;
    }
    layersList->setVisible(layers->isVisible());
    biomesList->setVisible(biomes->isVisible());
    propertiesList->setVisible(properties->isVisible());
    layersHelp->setVisible(layers->isVisible());
    biomesHelp->setVisible(biomes->isVisible());
    propertiesHelp->setVisible(properties->isVisible());
    gotoAndStop(label);

    if (getStage()->getFocus()->isFJ_Base() && m_currentTab != tab)
    {
        if (m_currentTab == 0)
            m_layersFocus = getStage()->getFocus()->asFJ_Base()->getId();
        else if (m_currentTab == 1)
            m_biomesFocus = getStage()->getFocus()->asFJ_Base()->getId();
        else if (m_currentTab == 2)
            m_propertiesFocus = getStage()->getFocus()->asFJ_Base()->getId();
    }

    m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
    m_root = activeTab;
    m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
    SetFocus(focus);
    if (focus >= 0)
        OnFocusChange(true);

    m_activeTab = tab;
    m_currentTab = tab;
}

FJ_SuperFlatMenu* FJ_SuperFlatMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SuperFlatMenu(renderNode);
}

unsigned int FJ_SuperFlatMenu::getType()
{
    return 39;
}
