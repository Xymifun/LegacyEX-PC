#include "game/ui/UIScene_SuperFlatPresets.hpp"
#include "game/core/StringIds.hpp"

#include <cstdio>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DefaultPresets.hpp"
#include "game/core/L10N.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/GameSettings.hpp"

namespace
{
struct DefaultPresetInfo
{
    unsigned int nameId;
    int icon;
    int preset;
};

DefaultPresetInfo s_defaultPresetInfo[] = {{IDS_PRESET_CLASSIC_FLAT, 2, DefaultPresets::CLASSIC},
                                           {IDS_PRESET_TUNNELERS_DREAM, 1, DefaultPresets::TUNNELERS_DREAM},
                                           {IDS_PRESET_WATER_WORLD, 9, DefaultPresets::WATER_WORLD},
                                           {IDS_PRESET_OVERWORLD, 31, DefaultPresets::OVERWORLD},
                                           {IDS_PRESET_SNOWY_KINGDOM, 78, DefaultPresets::SNOWY_KINGDOM},
                                           {IDS_PRESET_DESERT, 12, DefaultPresets::DESERT},
                                           {IDS_PRESET_REDSTONE_READY, 331, DefaultPresets::REDSTONE_READY},
                                           {static_cast<unsigned int>(-1), -1, -1}};
}

UIScene_SuperFlatPresets::UIScene_SuperFlatPresets(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer),
      m_generator(static_cast<SuperflatConfig*>(0)),
      m_functionPresetIds(),
      m_customPresets(),
      m_defaultPresets(),
      m_presetList(),
      m_presetTitle()
{
    SuperflatConfig* config = static_cast<SuperflatConfig**>(data)[1];
    m_generator = FlatGeneratorInfo(config, false);
    initialiseMovie();
    m_presetTitle.init(UIString(IDS_TOOLTIPS_PRESETS));
    m_presetList.init(0);
    m_presetList.UseIggyIcons();

    for (int i = 0; s_defaultPresetInfo[i].preset >= 0; ++i)
    {
        SuperflatConfig* preset = new SuperflatConfig();
        preset->makePreset(static_cast<DefaultPresets::Preset>(s_defaultPresetInfo[i].preset));
        m_defaultPresets.push_back(preset);
    }
    PopulatePresetList();
    m_selectedPreset = -1;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_SuperFlatPresets::~UIScene_SuperFlatPresets()
{
    for (std::vector<SuperflatConfig*>::iterator it = m_customPresets.begin(); it != m_customPresets.end(); ++it)
        delete *it;
    m_customPresets.clear();
    for (std::vector<SuperflatConfig*>::iterator it = m_defaultPresets.begin(); it != m_defaultPresets.end(); ++it)
        delete *it;
    m_defaultPresets.clear();
}

unsigned int UIScene_SuperFlatPresets::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_presetList.setupControl(this, root, std::string("PresetsList"));
        m_controls.push_back(&m_presetList);
    }
    {
        m_presetTitle.setupControl(this, root, std::string("PresetsTitle"));
        m_controls.push_back(&m_presetTitle);
    }
    return 1;
}

int UIScene_SuperFlatPresets::GetPresetCount() const
{
    return static_cast<int>(m_functionPresetIds.size() + m_customPresets.size() + m_defaultPresets.size());
}

int UIScene_SuperFlatPresets::SafeGetIndex(unsigned int count, int start, int index) const
{
    int end = start + static_cast<int>(count);
    if (start <= index && index < end)
        return index - start;
    return -1;
}

int UIScene_SuperFlatPresets::GetFunctionPresetId(int index) const
{
    return SafeGetIndex(static_cast<unsigned int>(m_functionPresetIds.size()), 0, index);
}

int UIScene_SuperFlatPresets::GetCustomPresetId(int index) const
{
    return SafeGetIndex(static_cast<unsigned int>(m_customPresets.size()), static_cast<int>(m_functionPresetIds.size()),
                        index);
}

int UIScene_SuperFlatPresets::GetDefaultPresetId(int index) const
{
    return SafeGetIndex(static_cast<unsigned int>(m_defaultPresets.size()),
                        static_cast<int>(m_functionPresetIds.size() + m_customPresets.size()), index);
}

std::wstring UIScene_SuperFlatPresets::GetDisplayName(int index) const
{
    int functionId = GetFunctionPresetId(index);
    int customId = GetCustomPresetId(index);
    int defaultId = GetDefaultPresetId(index);
    if (functionId == 0)
        return L10N::GetString(0x477A886A);
    if (customId >= 0)
        return m_customPresets[customId]->getDisplayname();
    if (defaultId >= 0)
        return L10N::GetString(static_cast<int>(s_defaultPresetInfo[defaultId].nameId));
    return L"[UNKNOWN_PRESET]";
}

void UIScene_SuperFlatPresets::PopulatePresetList()
{
    m_presetList.clearList();
    int i;
    for (i = 0; i < GetPresetCount(); ++i)
        m_presetList.AddNewItem(GetDisplayName(i), i, L"");
}

unsigned int UIScene_SuperFlatPresets::GetFunction(int function) const
{
    int selection = m_presetList.getCurrentSelection();
    int functionId = GetFunctionPresetId(selection);
    int customId = GetCustomPresetId(selection);
    int defaultId = GetDefaultPresetId(selection);
    if (functionId >= 0)
    {
        if (functionId == 0 && function == 0)
            return static_cast<unsigned int>(function);
        return static_cast<unsigned int>(-1);
    }
    if (customId >= 0 || defaultId >= 0)
    {
        if (function == 0)
            return 1;
        if (customId >= 0)
        {
            switch (function)
            {
            case 2:
            case 3:
                return static_cast<unsigned int>(function);
            }
        }
    }
    return static_cast<unsigned int>(-1);
}

int UIScene_SuperFlatPresets::GetTooltip(int function) const
{
    switch (GetFunction(function))
    {
    case 0:
        return IDS_SUPERFLAT_SAVE_PRESET;
    case 1:
        return IDS_SUPERFLAT_LOAD_PRESET;
    case 2:
        return IDS_SUPERFLAT_OVERWRITE;
    case 3:
        return IDS_SUPERFLAT_DELETE_PRESET;
    }
    return -1;
}

void UIScene_SuperFlatPresets::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, GetTooltip(0), IDS_TOOLTIPS_BACK, GetTooltip(2), GetTooltip(3), -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SuperFlatPresets::updateComponents()
{
}

SuperflatConfig* UIScene_SuperFlatPresets::GetSuperflatConfig(int index) const
{
    int functionId = GetFunctionPresetId(index);
    int customId = GetCustomPresetId(index);
    int defaultId = GetDefaultPresetId(index);
    if (functionId < 0)
    {
        if (customId >= 0)
            return m_customPresets[customId];
        if (defaultId >= 0)
            return m_defaultPresets[defaultId];
    }
    return 0;
}

int UIScene_SuperFlatPresets::LoadConfirmMessageCallback(void* data, int, C4JStorage::EMessageResult result)
{
    UIScene_SuperFlatPresets* scene = static_cast<UIScene_SuperFlatPresets*>(data);
    if (result == C4JStorage::EMessageResult_Accept)
    {
        scene->m_generator.setEqualTo(scene->GetSuperflatConfig(scene->m_selectedPreset));
        gConsoleUIController.NavigateBack(scene->m_playerId, false, EUIScene_None, EUILayer_Count);
    }
    return 0;
}

void UIScene_SuperFlatPresets::HandlePresetListPress(int function)
{
    int selection = m_presetList.getCurrentSelection();
    if (GetFunction(function) == 1)
    {
        m_selectedPreset = selection;
        int parameters[2];
        parameters[0] = IDS_CONFIRM_OK;
        parameters[1] = IDS_CONFIRM_CANCEL;
        gConsoleUIController.RequestAlertMessage(IDS_SUPERFLAT_LOAD_PRESET,
                                                 IDS_SUPERFLAT_UNSAVED_CONFIGURATION_LOAD_MESSAGE, parameters, 2,
                                                 m_playerId, LoadConfirmMessageCallback, this, 0, true);
    }
}

void UIScene_SuperFlatPresets::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuX:
    case EControllerActions_MenuY:
        if (pressed)
        {
            HandlePresetListPress(action);
            handled = true;
        }
        break;
    default:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_SuperFlatPresets::handleFocusChange(int, int childFocus)
{
    m_presetList.updateChildFocus(childFocus);
    updateTooltips();
}

void UIScene_SuperFlatPresets::handlePress(int controlId, int)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId == 0)
        HandlePresetListPress(0);
}

int UIScene_SuperFlatPresets::GetIcon(int index) const
{
    int functionId = GetFunctionPresetId(index);
    int customId = GetCustomPresetId(index);
    int defaultId = GetDefaultPresetId(index);
    if (functionId == 0)
        return Block::getId(Blocks::crafting_table);
    if (customId >= 0)
        return Item::getId(Items::painting);
    if (defaultId >= 0)
        return s_defaultPresetInfo[defaultId].icon;
    return Block::getId(Blocks::command_block);
}

void UIScene_SuperFlatPresets::customDraw(const char* name, fuiRect* rect)
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slot = -1;
    std::sscanf(name, "slot_0_%d", &slot);
    if (slot != -1)
        item = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(GetIcon(slot)), 1, 0));
    if (!item.get()->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item.get()->getItem()->isFoil(item), false, false);
}

std::wstring UIScene_SuperFlatPresets::getMoviePath()
{
    const wchar_t* path = L"SuperFlatPresets";
    return path;
}

int UIScene_SuperFlatPresets::getSceneType()
{
    return EUIScene_SuperFlatPresets;
}
