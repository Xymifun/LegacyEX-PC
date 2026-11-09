#include "game/ui/UIScene_SuperFlatMenu.hpp"
#include "game/core/StringIds.hpp"
#include "game/ui/FJ_SuperFlatMenu.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/L10N.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/core/GameSettings.hpp"

#include <cstdio>
#include <cstdlib>

UIScene_SuperFlatMenu::TickAction::~TickAction()
{
}

UIScene_SuperFlatMenu::MethodAction::MethodAction(Func function) : m_function(function)
{
}

UIScene_SuperFlatMenu::MethodAction::~MethodAction()
{
}

void UIScene_SuperFlatMenu::MethodAction::Execute(UIScene_SuperFlatMenu* scene)
{
    (scene->*m_function)();
}

UIScene_SuperFlatMenu::UIScene_SuperFlatMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer),
      m_tickActions(),
      m_layersTouch(),
      m_biomesTouch(),
      m_propertiesTouch(),
      m_paramMappings(),
      m_generatorInfo(NULL)
{
    initialiseMovie();
    m_generating = false;
    m_editingLayer = false;
    m_initData = static_cast<LaunchMoreOptionsMenuInitData*>(data);
    m_activeTab = eTabLayers;
    _init();
    m_config = new SuperflatConfig();
    LoadPreset(&m_initData->mSuperflatConfig);
}

UIScene_SuperFlatMenu::~UIScene_SuperFlatMenu()
{
    for (int i = 0; !SuperflatMenu::params[i].isTerminal(); ++i)
    {
        if (SuperflatMenu::params[i].isParent())
            delete m_paramMappings[i];
    }
    m_paramMappings.clear();
    delete m_config;
}

unsigned int UIScene_SuperFlatMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_layers.tab.setupControl(this, currentNode, std::string("LayersTabTitle"));
        m_controls.push_back(&m_layers.tab);
    }
    {
        m_biomes.tab.setupControl(this, currentNode, std::string("BiomesTabTitle"));
        m_controls.push_back(&m_biomes.tab);
    }
    {
        m_properties.tab.setupControl(this, currentNode, std::string("PropertiesTabTitle"));
        m_controls.push_back(&m_properties.tab);
    }
    {
        m_layers.panel.setupControl(this, currentNode, std::string("Layers"));
        m_controls.push_back(&m_layers.panel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_layers.panel.getFuiRenderNode();
        {
            m_layers.list.setupControl(this, currentNode, std::string("LayersList"));
            m_controls.push_back(&m_layers.list);
        }
        {
            m_layers.helpTitle.setupControl(this, currentNode, std::string("LayersHelpTitle"));
            m_controls.push_back(&m_layers.helpTitle);
        }
        {
            m_layers.help.setupControl(this, currentNode, std::string("LayersHelpHtml"));
            m_controls.push_back(&m_layers.help);
        }
        {
            m_layers.timer.setupControl(this, currentNode, std::string("LayersTimer"));
            m_controls.push_back(&m_layers.timer);
        }
        {
            m_layersTouch.setupControl(this, currentNode, std::string("TouchTabLayers"));
            m_controls.push_back(&m_layersTouch);
        }
        currentNode = parentNode;
    }
    {
        m_biomes.panel.setupControl(this, currentNode, std::string("Biomes"));
        m_controls.push_back(&m_biomes.panel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_biomes.panel.getFuiRenderNode();
        {
            m_biomes.list.setupControl(this, currentNode, std::string("BiomesList"));
            m_controls.push_back(&m_biomes.list);
        }
        {
            m_biomes.helpTitle.setupControl(this, currentNode, std::string("BiomesHelpTitle"));
            m_controls.push_back(&m_biomes.helpTitle);
        }
        {
            m_biomes.help.setupControl(this, currentNode, std::string("BiomesHelpHtml"));
            m_controls.push_back(&m_biomes.help);
        }
        {
            m_biomes.timer.setupControl(this, currentNode, std::string("BiomesTimer"));
            m_controls.push_back(&m_biomes.timer);
        }
        {
            m_biomesTouch.setupControl(this, currentNode, std::string("TouchTabBiomes"));
            m_controls.push_back(&m_biomesTouch);
        }
        currentNode = parentNode;
    }
    {
        m_properties.panel.setupControl(this, currentNode, std::string("Properties"));
        m_controls.push_back(&m_properties.panel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_properties.panel.getFuiRenderNode();
        {
            m_properties.list.setupControl(this, currentNode, std::string("PropertiesList"));
            m_controls.push_back(&m_properties.list);
        }
        {
            m_properties.helpTitle.setupControl(this, currentNode, std::string("PropertiesHelpTitle"));
            m_controls.push_back(&m_properties.helpTitle);
        }
        {
            m_properties.help.setupControl(this, currentNode, std::string("PropertiesHelpHtml"));
            m_controls.push_back(&m_properties.help);
        }
        {
            m_properties.timer.setupControl(this, currentNode, std::string("PropertiesTimer"));
            m_controls.push_back(&m_properties.timer);
        }
        {
            m_propertiesTouch.setupControl(this, currentNode, std::string("TouchTabProperties"));
            m_controls.push_back(&m_propertiesTouch);
        }
        currentNode = parentNode;
    }
    return 1;
}

void UIScene_SuperFlatMenu::PopulateBiomesList()
{
    SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
    int index = 0;
    for (int i = 0; biomes[i].mBiome >= 0; ++i, ++index)
        m_biomes.list.AddNewItem(biomes[i].getName(), index, false, L"");
}

int UIScene_SuperFlatMenu::ParentParam(int index)
{
    for (int i = index; i >= 0; --i)
    {
        if (SuperflatMenu::params[i].isParent())
            return i;
    }
    return -1;
}

void UIScene_SuperFlatMenu::CreateParamMappings()
{
    for (int i = 0; !SuperflatMenu::params[i].isTerminal(); ++i)
    {
        SuperflatMenu::Param* param = &SuperflatMenu::params[i];
        if (param->isParent())
            m_paramMappings[i] = param->getFunc()(i, &m_properties.list);
        else
            m_paramMappings[i] = m_paramMappings[ParentParam(i)];
    }
}

void UIScene_SuperFlatMenu::PopulateParamsList()
{
    int index = 0;
    for (SuperflatMenu::Param* param = SuperflatMenu::params; !param->isTerminal(); ++param, ++index)
    {
        if (param->isParent())
        {
            m_properties.list.AddNewCheckbox(L10N::GetString(param->getName()), index, false);
        }
        else if (param->isChild())
        {
            m_properties.list.AddNewSlider(L"", index, param->getMinValue(), param->getMaxValue(), param->getMinValue(),
                                           false, 7, 3);
        }
    }
}

void UIScene_SuperFlatMenu::_init()
{
    {
        m_layers.tab.init(UIString(IDS_LAYERS));
        m_layers.helpTitle.init(UIString(L" "));
        m_layers.list.init(0);
        m_layers.list.UseIggyIcons();
    }
    {
        m_biomes.tab.init(UIString(IDS_BIOMES));
        m_biomes.helpTitle.init(UIString(IDS_ACHIEVEMENTS));
        m_biomes.list.init(1);
        m_biomes.list.UseIggyIcons();
        PopulateBiomesList();
    }
    {
        m_properties.tab.init(UIString(IDS_PROPERTIES));
        m_properties.helpTitle.init(UIString(IDS_ACHIEVEMENTS));
        m_properties.list.init(2);
        CreateParamMappings();
        PopulateParamsList();
    }
    m_layers.timer.setVisible(false);
    m_biomes.timer.setVisible(false);
    m_properties.timer.setVisible(false);
    m_layersTouch.init(3);
    m_biomesTouch.init(4);
    m_propertiesTouch.init(5);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SuperFlatMenu::setLayerButton(int index, const FlatLayerInfo& layer)
{
    unsigned int count = m_layers.list.getItemCount();
    if (index <= (int)count)
    {
        if (index >= (int)count)
            m_layers.list.AddNewItem(layer.getNameText(), layer.getHeightText(), index, L"");
        else
            m_layers.list.SetItem(index, layer.getNameText(), layer.getHeightText(), L"");
    }
}

void UIScene_SuperFlatMenu::PopulateLayersList()
{
    std::vector<FlatLayerInfo> layers;
    m_generatorInfo.getLayers(layers);
    m_layers.list.clearList();
    int index = 0;
    for (std::vector<FlatLayerInfo>::reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it)
    {
        setLayerButton(index, *it);
        ++index;
    }
}

void UIScene_SuperFlatMenu::UpdateBiomesList()
{
    SuperflatMenu::BiomeInfo* biome = SuperflatMenu::getBiomeInfoA();
    int index = 0;
    while (biome->mBiome >= 0)
    {
        bool checked = m_generatorInfo.getBiome() == biome->mBiome;
        if (!checked)
        {
            if (biome->mBiome == Biome::eBiomeIDs_Ocean)
            {
                if (m_generatorInfo.getBiome() == Biome::eBiomeIDs_DeepOcean)
                    checked = true;
            }
        }
        m_biomes.list.SetCheckbox(index, checked);
        ++biome;
        ++index;
    }
}

void UIScene_SuperFlatMenu::UpdateParamsList()
{
    for (int i = 0; !SuperflatMenu::params[i].isTerminal(); ++i)
    {
        if (SuperflatMenu::params[i].isParent())
            m_paramMappings[i]->readConfig(m_generatorInfo);
    }
}

void UIScene_SuperFlatMenu::LoadPreset(SuperflatConfig* config)
{
    m_presetsFlag = false;
    *m_config = *config;
    m_generatorInfo = FlatGeneratorInfo(m_config);
    PopulateLayersList();
    UpdateBiomesList();
    UpdateParamsList();
}

void UIScene_SuperFlatMenu::SetActiveTab(eTabs tab)
{
    static_cast<FJ_SuperFlatMenu*>(m_movie->getRootNode()->getFuiNode())->SetActiveTab(tab);
    m_activeTab = tab;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SuperFlatMenu::handleTouchInput(unsigned int, int, int, int touchBox, bool pressed, bool, bool)
{
    if (pressed && !m_editingLayer)
    {
        switch (touchBox)
        {
        case 3:
        case 4:
        case 5:
        {
            int tab = touchBox - 3;
            if (m_activeTab != tab)
            {
                m_activeTab = tab;
                updateTooltips();
                SetActiveTab(static_cast<eTabs>(m_activeTab));
            }
        }
        break;
        }
    }
}

UIControl* UIScene_SuperFlatMenu::GetMainPanel()
{
    switch (m_activeTab)
    {
    case eTabBiomes:
        return &m_biomes.panel;
    case eTabProperties:
        return &m_properties.panel;
    default:
        return &m_layers.panel;
    }
}

void UIScene_SuperFlatMenu::updateTooltips()
{
    int primaryAction = -1;
    int moveLayerAction = -1;
    int secondaryAction = -1;
    int pageUpAction = -1;
    int pageDownAction = -1;
    int disabledAction = -1;
    int layerAction = -1;
    int moveUpDownAction = -1;
    if (!m_editingLayer)
    {
        disabledAction = -2;
        layerAction = IDS_TOOLTIPS_CHANGE_TAB;
        secondaryAction = IDS_TOOLTIPS_PRESETS;
    }
    switch (m_activeTab)
    {
    case eTabLayers:
        if (controlHasFocus(m_layers.list.getId()))
        {
            unsigned int selection = m_layers.list.getCurrentSelection();
            unsigned int count = m_layers.list.getItemCount();
            bool lastLayer = selection == count - 1;
            if (m_editingLayer)
            {
                primaryAction = IDS_TOOLTIPS_PLACE;
                pageUpAction = IDS_TOOLTIPS_PAGE_UP;
                pageDownAction = IDS_TOOLTIPS_PAGE_DOWN;
                moveUpDownAction = IDS_TOOLTIPS_MOVE_UPDOWN;
            }
            else
            {
                primaryAction = IDS_SUPERFLAT_ADD_LAYER;
                if (!lastLayer)
                {
                    primaryAction = IDS_TOOLTIPS_LAYER_OPTIONS;
                    moveLayerAction = IDS_SUPERFLAT_MOVE_LAYER;
                }
            }
        }
        break;
    case eTabBiomes:
    case eTabProperties:
        primaryAction = IDS_TOOLTIPS_SELECT;
        break;
    }
    gConsoleUIController.SetTooltips(0, primaryAction, IDS_TOOLTIPS_BACK, moveLayerAction, secondaryAction,
                                     pageUpAction, pageDownAction, disabledAction, layerAction, -1, -1, -1, -1, -1, -1,
                                     -1, moveUpDownAction, false, false);
}

void UIScene_SuperFlatMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_SuperFlatMenu::tick()
{
    UIScene::tick();
    for (std::vector<TickAction*>::iterator it = m_tickActions.begin(); it != m_tickActions.end(); ++it)
    {
        TickAction* action = *it;
        action->Execute(this);
        delete action;
    }
    m_tickActions.clear();
}

int UIScene_SuperFlatMenu::GetLayerIndex(int index)
{
    return m_generatorInfo.numLayers() - index - 1;
}

void UIScene_SuperFlatMenu::UpdateLayersHelpText()
{
    int index = GetLayerIndex(m_layers.list.getCurrentSelection());
    FlatLayerInfo layer = m_generatorInfo.getLayer(index);
    m_layers.helpTitle.setLabel(UIString(layer.getNameText()), false, false);
    m_layers.help.setLabel(UIString(layer.getDescriptionText()), false, false);
}

void UIScene_SuperFlatMenu::UpdateBiomeHelpText()
{
    SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
    SuperflatMenu::BiomeInfo& biome = biomes[m_biomes.list.getCurrentSelection()];
    m_biomes.helpTitle.setLabel(UIString(biome.getName()), false, false);
    std::wstring description(L10N::GetString(biome.mDescription));
    m_biomes.help.setLabel(UIString(description.c_str()), false, false);
}

void UIScene_SuperFlatMenu::UpdateParamHelpText()
{
    const int selection = m_properties.list.getCurrentSelection();
    const int parent = ParentParam(selection);
    const SuperflatMenu::Param& parentParam = SuperflatMenu::params[parent];
    const SuperflatMenu::Param& selectedParam = SuperflatMenu::params[selection];
    m_properties.helpTitle.setLabel(UIString(parentParam.getName()), false, false);

    std::wstring text(L"");
    text.append(L10N::GetString(parentParam.getDesc()));
    if (parent != selection)
    {
        text.append(L"\n\n");
        text.append(L10N::GetString(selectedParam.getName()));
        text.append(L"\n\n");
        text.append(L10N::GetString(selectedParam.getDesc()));
    }
    m_properties.help.setLabel(UIString(text), false, false);
}

void UIScene_SuperFlatMenu::handleReload()
{
    int selection = m_pendingSelection;
    _init();
    PopulateLayersList();
    UpdateBiomesList();
    UpdateParamsList();
    static_cast<FJ_SuperFlatMenu*>(m_movie->getRootNode()->getFuiNode())->SetActiveTab(m_activeTab);
    m_pendingSelection = selection;
    switch (m_activeTab)
    {
    case eTabLayers:
        m_layers.list.setCurrentSelection(m_pendingSelection, false, true);
        UpdateLayersHelpText();
        break;
    case eTabBiomes:
        m_biomes.list.setCurrentSelection(m_pendingSelection, false, true);
        UpdateBiomeHelpText();
        break;
    case eTabProperties:
        m_properties.list.setCurrentSelection(m_pendingSelection, false, true);
        UpdateParamHelpText();
        break;
    }
}

void UIScene_SuperFlatMenu::ExecuteOnTick(MethodAction::Func function)
{
    m_tickActions.push_back(new MethodAction(function));
}

void UIScene_SuperFlatMenu::MoveCurrentLayer(int amount)
{
    if (amount == 0)
        return;
    int direction = (amount > 0) ? -1 : 1;
    int selection = m_layers.list.getCurrentSelection();
    int moved = selection + direction * std::abs(amount);
    int layers = m_generatorInfo.numLayers();
    int maxIndex = layers - 1;
    int target = (layers - moved) - 1;
    int limit = (layers - selection) - 1;
    target = (maxIndex < target) ? maxIndex : target;
    target = (target < 1) ? 1 : target;
    limit = (maxIndex < limit) ? maxIndex : limit;
    limit = (limit < 1) ? 1 : limit;
    direction = -direction;
    int selectedLayer = (layers - target) - 1;
    if (target == limit || limit == 0)
    {
        ExecuteOnTick<const SoundEvent*>(&UIScene_SuperFlatMenu::PlaySound, SoundEvents::UI_CRAFT_FAIL);
        return;
    }
    ExecuteOnTick<const SoundEvent*>(&UIScene_SuperFlatMenu::PlaySound, SoundEvents::UI_PRESS);
    FlatLayerInfo moving = m_generatorInfo.getLayer(limit);
    for (int i = limit; i != target; i += direction)
        m_generatorInfo.setLayer(i, m_generatorInfo.getLayer(i + direction));
    m_generatorInfo.setLayer(target, moving);
    ExecuteOnTick(&UIScene_SuperFlatMenu::PopulateLayersList);
    ExecuteOnTick<int>(&UIScene_SuperFlatMenu::SetSelectedLayer, selectedLayer);
}

void UIScene_SuperFlatMenu::NavActiveTab(int direction)
{
    int tab;
    while (true)
    {
        tab = direction + m_activeTab;
        if (tab < 0)
            direction += 3;
        else if (tab >= 3)
            direction -= 3;
        else
            break;
    }
    SetActiveTab((eTabs)tab);
}

int UIScene_SuperFlatMenu::CbGenerationWarning(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatMenu* scene = static_cast<UIScene_SuperFlatMenu*>(context);
    int ret = 0;
    switch (result)
    {
    case C4JStorage::EMessageResult_Cancelled:
        break;
    case C4JStorage::EMessageResult_Accept:
        scene->ReturnResults(false);
        break;
    case C4JStorage::EMessageResult_Decline:
        break;
    default:
        ret = -1;
        break;
    }
    scene->m_generating = false;
    return ret;
}

void UIScene_SuperFlatMenu::ReturnResults(bool accepted)
{
    unsigned int warning = SuperflatMenu::CustomSuperflatBlocks::verify(m_initData, m_config);
    if (accepted && warning != 0)
    {
        m_generating = true;
        int params[2] = {static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL)};
        gConsoleUIController.RequestAlertMessage(static_cast<int>(IDS_SUPERFLAT_CONFIGURATION_PROBLEMS),
                                                 static_cast<int>(warning), params, 2, m_playerId,
                                                 &UIScene_SuperFlatMenu::CbGenerationWarning, this, NULL, true);
        return;
    }
    SuperflatMenu::CustomSuperflatBlocks::sanitise(m_generatorInfo);
    m_initData->mSuperflatConfig = *m_config;
    SuperflatMenu::PresetCache::SetCachedPreset(m_playerId, *m_config);
    navigateBack();
}

void UIScene_SuperFlatMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                        bool& handled)
{
    if (m_generating)
        return;
    if (!m_tickActions.empty())
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuY:
        if (!m_editingLayer && pressed)
        {
            ExecuteOnTick(&UIScene_SuperFlatMenu::OpenPresetsDialog);
            handled = true;
            return;
        }
        break;
    case EControllerActions_MenuX:
    {
        bool last = m_layers.list.getCurrentSelection() == m_layers.list.getItemCount() - 1;
        if (!last && !m_editingLayer && m_layers.list.hasFocus() && pressed)
        {
            m_editingLayer = true;
            updateTooltips();
            handled = true;
            return;
        }
    }
    break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        if (m_editingLayer && pressed)
        {
            switch (action)
            {
            case EControllerActions_MenuDown:
                MoveCurrentLayer(-1);
                return;
            case EControllerActions_MenuPageUp:
                MoveCurrentLayer(10);
                return;
            }
            MoveCurrentLayer(action == EControllerActions_MenuPageDown ? -10 : 1);
            return;
        }
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        return;
    case EControllerActions_MenuOk:
        if (m_editingLayer && pressed)
        {
            m_editingLayer = false;
            handled = true;
            updateTooltips();
            return;
        }
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        return;
    case EControllerActions_MenuB:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        if (!m_editingLayer)
        {
            sendInputToMovie(action, repeat, pressed, released);
            handled = true;
        }
        return;
    case EControllerActions_MenuRightScroll:
    case EControllerActions_MenuLeftScroll:
        if (!m_editingLayer && pressed)
        {
            NavActiveTab(action == EControllerActions_MenuRightScroll ? 1 : -1);
            updateTooltips();
            handled = true;
            return;
        }
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            ReturnResults(true);
            handled = true;
        }
        break;
    default:
        break;
    }
}

int UIScene_SuperFlatMenu::CbLayerOperation(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatMenu* scene = static_cast<UIScene_SuperFlatMenu*>(context);
    int ret = 0;
    switch (result)
    {
    case C4JStorage::EMessageResult_Cancelled:
        break;
    case C4JStorage::EMessageResult_Accept:
        scene->ExecuteOnTick<bool>(&UIScene_SuperFlatMenu::OpenEditLayerDialog, false);
        break;
    case C4JStorage::EMessageResult_Decline:
        if (scene->m_generatorInfo.getHeight() < 0x80)
            scene->ExecuteOnTick<bool>(&UIScene_SuperFlatMenu::OpenEditLayerDialog, true);
        else
            scene->ExecuteOnTick(&UIScene_SuperFlatMenu::OpenMaxHeightDialog);
        break;
    case C4JStorage::EMessageResult_ThirdOption:
        scene->ExecuteOnTick(&UIScene_SuperFlatMenu::DeleteLayer);
        break;
    default:
        ret = -1;
        break;
    }
    scene->m_generating = false;
    return ret;
}

void UIScene_SuperFlatMenu::ChangeBiome(int biome)
{
    m_generatorInfo.setBiome(SuperflatMenu::getBiomeInfoA()[biome].mBiome);
    ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateBiomesList);
}

void UIScene_SuperFlatMenu::HandleChangeParamList(int index)
{
    ExecuteOnTick<int>(&UIScene_SuperFlatMenu::WritePropertyChanges, index);
}

void UIScene_SuperFlatMenu::handlePress(int controlId, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (controlId)
    {
    case 0:
        if (!m_editingLayer)
        {
            bool last = m_layers.list.getCurrentSelection() == m_layers.list.getItemCount() - 1;
            if (last)
            {
                if (m_generatorInfo.getHeight() < 0x80)
                {
                    ExecuteOnTick<bool>(&UIScene_SuperFlatMenu::OpenEditLayerDialog, true);
                    return;
                }
                ExecuteOnTick(&UIScene_SuperFlatMenu::OpenMaxHeightDialog);
                return;
            }
            int params[3] = {static_cast<int>(IDS_SUPERFLAT_EDIT_LAYER), static_cast<int>(IDS_SUPERFLAT_ADD_LAYER),
                             static_cast<int>(IDS_SUPERFLAT_DELETE_LAYER)};
            gConsoleUIController.RequestAlertMessage(static_cast<int>(IDS_SUPERFLAT_LAYER_OPTIONS_TITLE),
                                                     static_cast<int>(0x78a7e3b3U), params, 3, m_playerId,
                                                     &UIScene_SuperFlatMenu::CbLayerOperation, this, NULL, true);
            m_generating = true;
            return;
        }
        return;
    case 1:
        ChangeBiome(childId);
        return;
    case 2:
        HandleChangeParamList(childId);
        return;
    }
}

void UIScene_SuperFlatMenu::handleCheckboxElementToggled(int first, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (first == 2)
        HandleChangeParamList(second);
}

void UIScene_SuperFlatMenu::handleSliderElementMove(int first, int second, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (first == 2)
        HandleChangeParamList(second);
}

void UIScene_SuperFlatMenu::handleSelectionChanged(int selection)
{
    if (m_activeTab == eTabBiomes)
    {
        SuperflatMenu::BiomeInfo* biomes = SuperflatMenu::getBiomeInfoA();
        m_biomes.help.setLabel(UIString(biomes[selection].mDescription), false, false);
        return;
    }
    if (m_activeTab == eTabProperties)
    {
        SuperflatMenu::Param* param = &SuperflatMenu::params[selection];
        if (param->isParent())
        {
            m_properties.help.setLabel(UIString(param->getDesc()), false, false);
            return;
        }
        std::wstring text = std::wstring(L10N::GetString(SuperflatMenu::params[ParentParam(selection)].getDesc()))
                            + L"<br/>" + std::wstring(L10N::GetString(param->getDesc()));
        m_properties.help.setLabel(UIString(text), false, false);
        return;
    }
    if (m_activeTab == eTabLayers)
        controlHasFocus(&m_layers.list);
}

void UIScene_SuperFlatMenu::handleFocusChange(int focus, int childFocus)
{
    UIControl_List* list;
    switch (focus)
    {
    case eTabLayers:
        list = &m_layers.list;
        break;
    case eTabBiomes:
        list = &m_biomes.list;
        break;
    case eTabProperties:
        list = &m_properties.list;
        break;
    default:
        list = NULL;
        break;
    }
    if (list != NULL)
        list->updateChildFocus(childFocus);
    m_pendingSelection = list->getListIndex(childFocus);
    switch (focus)
    {
    case eTabLayers:
        ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateLayersHelpText);
        break;
    case eTabBiomes:
        ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateBiomeHelpText);
        break;
    case eTabProperties:
        ExecuteOnTick(&UIScene_SuperFlatMenu::UpdateParamHelpText);
        break;
    }
    updateTooltips();
}

void UIScene_SuperFlatMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    if (hadFocus)
    {
        SetActiveTab(eTabLayers);
        PopulateLayersList();
        UpdateBiomesList();
        UpdateParamsList();
    }
}

void UIScene_SuperFlatMenu::customDraw(const char* name, fuiRect* rect)
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slot;
    if (m_activeTab == eTabLayers)
    {
        slot = -1;
        std::sscanf(name, "slot_0_%d", &slot);
        if (slot != -1)
        {
            FlatLayerInfo layer = m_generatorInfo.getLayer(GetLayerIndex(slot));
            if (layer.getBlock() != Blocks::air)
            {
                int itemId = layer.getId();
                if (itemId >= 256 && itemId < 1024)
                    itemId += 768;
                item = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(itemId), 1, layer.getData()));
            }
        }
    }
    if (m_activeTab == eTabBiomes)
    {
        slot = -1;
        std::sscanf(name, "slot_1_%d", &slot);
        if (slot != -1)
            item = SuperflatMenu::getBiomeInfoA()[slot].mItem;
    }
    if (!item.get()->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item.get()->getItem()->isFoil(item), false, false);
}

void UIScene_SuperFlatMenu::SetSelectedLayer(int index)
{
    if (m_layers.list.hasFocus() && (int)m_layers.list.getItemCount() > index)
        m_layers.list.setCurrentSelection(index, false, true);
}

void UIScene_SuperFlatMenu::DeleteLayer()
{
    int index = GetLayerIndex(m_layers.list.getCurrentSelection());
    m_generatorInfo.remLayer(index);
    PopulateLayersList();
}

void UIScene_SuperFlatMenu::WritePropertyChanges(int index)
{
    m_paramMappings[index]->writeConfig(m_generatorInfo);
}

void UIScene_SuperFlatMenu::PlaySound(const SoundEvent* sound)
{
    gConsoleUIController.PlayUISFX(sound);
}

void UIScene_SuperFlatMenu::OpenPresetsDialog()
{
    struct PresetsData
    {
        bool flag;
        SuperflatConfig* config;
    };
    PresetsData* data = new PresetsData();
    data->flag = m_presetsFlag;
    data->config = m_config;
    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SuperFlatPresets, data, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}

void UIScene_SuperFlatMenu::OpenEditLayerDialog(bool insert)
{
    struct EditLayerData
    {
        bool insert;
        int index;
        SuperflatConfig* config;
    };
    EditLayerData* data = new EditLayerData();
    data->config = m_config;
    data->insert = insert;
    data->index = (m_generatorInfo.numLayers() - 1) - m_layers.list.getCurrentSelection();
    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SuperFlatEditLayer, data, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}

void UIScene_SuperFlatMenu::OpenMaxHeightDialog()
{
    int params[1] = {static_cast<int>(IDS_CONFIRM_OK)};
    gConsoleUIController.RequestAlertMessage(static_cast<int>(0xf2637855U), static_cast<int>(0x373ccbf0U), params, 1,
                                             m_playerId, NULL, NULL, NULL, true);
}

std::wstring UIScene_SuperFlatMenu::getMoviePath()
{
    return std::wstring(L"SuperFlatMenu");
}

int UIScene_SuperFlatMenu::getSceneType()
{
    return EUIScene_SuperFlatMenu;
}

template void UIScene_SuperFlatMenu::ExecuteOnTick<const SoundEvent*>(
    MethodActionArg1<const SoundEvent*>::Func function, const SoundEvent* value);
template void UIScene_SuperFlatMenu::ExecuteOnTick<int>(MethodActionArg1<int>::Func function, int value);
template void UIScene_SuperFlatMenu::ExecuteOnTick<bool>(MethodActionArg1<bool>::Func function, bool value);
