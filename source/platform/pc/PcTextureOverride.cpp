#include "platform/pc/PcTextureOverride.hpp"

#include <algorithm>
#include <cstdio>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "platform/pc/PcSettings.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"

extern ConsoleUIController gConsoleUIController;

namespace
{
bool skipOverride = false;
bool overriding = false;
unsigned int worldPack = 0;
bool defaultMusic = false;
bool miniGameMusic = false;

const int CHECKBOX_ID = 101;
const int LIST_ID = 102;
const int DONE_ID = 103;
const int MUSIC_ID = 104;
const int MINIGAME_ID = 105;

bool MapMusic()
{
    return overriding && !miniGameMusic && Minecraft::InMiniGame(eMiniGameId_Any, false);
}

void MakeCheckBox(UIScene* scene, fuiRenderNode* panel, const char* name, UIControl_CheckBox* box,
                  std::vector<UIControl*>* controls)
{
    FJ_FuiNode* made = FJ_Factory::createObject("fourj.Checkbox.FJ_CheckBox_Normal", panel, true, NULL);
    if (made == NULL || made->m_pRenderNode == NULL || panel->getFuiNode() == NULL)
        return;
    made->m_pRenderNode->PcSetName(name);
    panel->getFuiNode()->addChild(made);
    box->setupControl(scene, panel, std::string(name));
    controls->push_back(box);
}

struct PackOrder
{
    int sort;
    unsigned int id;
};

bool SortPacks(const PackOrder& a, const PackOrder& b)
{
    return b.sort < a.sort;
}
}

unsigned int PcTextureOverrideApply(unsigned int packId)
{
    defaultMusic = PcSettingsDefaultMusic();
    miniGameMusic = PcSettingsMiniGameMusicOverride();
    worldPack = packId;
    overriding = false;
    if (skipOverride || !PcSettingsTextureOverride())
        return packId;
    unsigned int pack = PcSettingsTextureOverridePack();
    if (Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(pack) == NULL)
        return packId;
    overriding = pack != packId;
    return pack;
}

void PcTextureOverrideSkip(bool skip)
{
    skipOverride = skip;
}

bool PcTextureOverrideActive()
{
    return overriding;
}

unsigned int PcTextureOverrideWorldPack(unsigned int selectedId)
{
    return overriding ? worldPack : selectedId;
}

bool PcTextureOverrideGameMusic()
{
    return defaultMusic || MapMusic();
}

TexturePack* PcTextureOverrideMusicPack()
{
    if (PcTextureOverrideGameMusic())
        return TexturePackRepository::defaultTexturePack;
    return Minecraft::GetInstance()->mTexturePackRepository->getSelected();
}

unsigned int PcTextureOverrideThemePack(unsigned int selectedId)
{
    if (defaultMusic)
        return 0;
    if (overriding
        && (!miniGameMusic || !Minecraft::GetInstance()->mTexturePackRepository->getSelected()->hasAudio()))
        return worldPack;
    return selectedId;
}

UIScene_PcTextureOverride::UIScene_PcTextureOverride(int player, void*, UILayer* layer)
    : UIScene(player, layer), m_slot(0), m_pendingSlot(-1), m_enabled(PcSettingsTextureOverride()),
      m_defaultMusic(PcSettingsDefaultMusic()), m_miniGameMusic(PcSettingsMiniGameMusicOverride()), m_changed(false),
      m_left(false)
{
    initialiseMovie();
    if (Minecraft::GetInstance()->mLevel != NULL)
        m_title.init(UIString(L"Texture Override (applies to next world)"));
    else
        m_title.init(UIString(L"Texture Override"));
    m_enableBox.init(UIString(L"Enable Override"), CHECKBOX_ID, m_enabled);
    if (m_miniGameBox.getFuiRenderNode() != NULL)
        m_miniGameBox.init(UIString(L"Minigame Music Override"), MINIGAME_ID, m_miniGameMusic);
    if (m_musicBox.getFuiRenderNode() != NULL)
        m_musicBox.init(UIString(L"Default Music"), MUSIC_ID, m_defaultMusic);
    m_packList.init(std::wstring(L10N::GetString(IDS_DLC_MENU_TEXTUREPACKS)), LIST_ID);
    m_doneButton.init(UIString(L10N::GetString(IDS_DONE)), DONE_ID);
    m_packName.init(UIString(L""));
    m_packDesc.init(UIString(L""));
    m_nameBox.setVisible(false);
    m_modeSlider.setVisible(false);
    m_difficultySlider.setVisible(false);
    m_moreButton.setVisible(false);
    FillPacks();
    Layout();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_PcTextureOverride::~UIScene_PcTextureOverride()
{
    if (!m_left && m_changed)
    {
        PcSettingsSetTextureOverride(m_enabled);
        PcSettingsSetDefaultMusic(m_defaultMusic);
        PcSettingsSetMiniGameMusicOverride(m_miniGameMusic);
        if (m_slot >= 0 && m_slot < (int)m_packs.size())
            PcSettingsSetTextureOverridePack(m_packs[m_slot]);
        PcSettingsSave();
    }
}

std::wstring UIScene_PcTextureOverride::getMoviePath()
{
    return std::wstring(L"CreateWorldMenu");
}

unsigned int UIScene_PcTextureOverride::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_mainPanel.setupControl(this, root, std::string("MainPanel"));
    m_controls.push_back(&m_mainPanel);
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_title.setupControl(this, panel, std::string("WorldName"));
    m_controls.push_back(&m_title);
    m_nameBox.setupControl(this, panel, std::string("EditWorldName"));
    m_controls.push_back(&m_nameBox);
    m_modeSlider.setupControl(this, panel, std::string("GameModeToggle"));
    m_controls.push_back(&m_modeSlider);
    m_difficultySlider.setupControl(this, panel, std::string("Difficulty"));
    m_controls.push_back(&m_difficultySlider);
    m_moreButton.setupControl(this, panel, std::string("MoreOptions"));
    m_controls.push_back(&m_moreButton);
    m_enableBox.setupControl(this, panel, std::string("CheckboxOnline"));
    m_controls.push_back(&m_enableBox);
    MakeCheckBox(this, panel, "PcMiniGameMusic", &m_miniGameBox, &m_controls);
    MakeCheckBox(this, panel, "PcDefaultMusic", &m_musicBox, &m_controls);
    m_packList.setupControl(this, panel, std::string("TexturePackSelector"));
    m_controls.push_back(&m_packList);
    m_doneButton.setupControl(this, panel, std::string("NewWorld"));
    m_controls.push_back(&m_doneButton);
    m_packName.setupControl(this, panel, std::string("TexturePackName"));
    m_controls.push_back(&m_packName);
    m_packDesc.setupControl(this, panel, std::string("TexturePackDescription"));
    m_controls.push_back(&m_packDesc);
    m_packIcon.setupControl(this, panel, std::string("Icon"));
    m_controls.push_back(&m_packIcon);
    m_packComparison.setupControl(this, panel, std::string("ComparisonPic"));
    m_controls.push_back(&m_packComparison);
    return 1;
}

int UIScene_PcTextureOverride::getSceneType()
{
    return EUIScene_PcTextureOverride;
}

void UIScene_PcTextureOverride::FillPacks()
{
    TexturePackRepository* repo = Minecraft::GetInstance()->mTexturePackRepository;
    int count = repo->getTexturePackCount();
    std::vector<PackOrder> order;
    int sort = 999;
    for (int i = 0; i < count; i++)
    {
        TexturePack* pack = repo->getTexturePackByIndex(i);
        PackOrder o;
        o.id = pack->getId();
        if (i == 0)
            o.sort = 1000;
        else
        {
            DLC_INFO* info = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(o.id);
            if (info != NULL)
                o.sort = info->SortIndex;
            else
            {
                o.id = ((DLCTexturePack*)pack)->getDLCInfoParentPack()->GetPackId();
                o.sort = sort--;
            }
        }
        order.push_back(o);
    }
    std::sort(order.begin(), order.end(), SortPacks);

    unsigned int chosen = PcSettingsTextureOverridePack();
    for (size_t i = 0; i < order.size(); i++)
    {
        TexturePack* pack = repo->getTexturePackById(order[i].id);
        if (pack == NULL)
            continue;
        unsigned int size = 0;
        const void* icon = pack->getPackIcon(size);
        if (size == 0 || icon == NULL)
            continue;
        wchar_t name[64];
        swprintf(name, 64, L"tpack%08x", pack->getId());
        registerSubstitutionTexture(name, (unsigned char*)icon, size, false);
        if (pack->getId() == chosen)
            m_slot = (int)m_packs.size();
        m_packList.addPack((int)m_packs.size(), name, false);
        m_packs.push_back(pack->getId());
    }
    if (m_packs.empty())
        return;
    m_packList.selectSlot(m_slot);
    ShowPack(m_packs[m_slot]);
}

void UIScene_PcTextureOverride::ShowPack(unsigned int packId)
{
    TexturePack* pack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(packId);
    if (pack == NULL)
        return;
    m_packName.setLabel(UIString(pack->getName()), false, false);
    m_packDesc.setLabel(UIString(pack->getDesc1()), false, false);
    wchar_t name[64];
    swprintf(name, 64, L"tpack%08x", pack->getId());
    m_packIcon.setTextureName(name, true);
    unsigned int size = 0;
    const void* pic = pack->getPackComparison(size);
    if (size == 0 || pic == NULL)
    {
        m_packComparison.setTextureName(L"", true);
        return;
    }
    swprintf(name, 64, L"texturePackComparison%08x", pack->getId());
    registerSubstitutionTexture(name, (unsigned char*)pic, size, false);
    m_packComparison.setTextureName(name, true);
}

void UIScene_PcTextureOverride::Layout()
{
    const float PANEL_TOP = 40.0f;
    const float PANEL_BOTTOM = 378.0f;
    fuiRenderNodeTimeline* panel = (fuiRenderNodeTimeline*)m_mainPanel.getFuiRenderNode()->asTimeline();
    if (panel == NULL)
        return;
    for (int i = 0; i < panel->getNumChildren(); i++)
    {
        fuiRenderNode* n = panel->getChildAt(i);
        const char* name = n->getName();
        if ((name == NULL || name[0] == 0) && n->getX() == 0.0f && n->getY() == 0.0f)
        {
            n->setY(PANEL_TOP);
            n->setHeight(PANEL_BOTTOM - PANEL_TOP);
            break;
        }
    }
    m_title.getFuiRenderNode()->setY(PANEL_TOP + 22.0f);
    m_enableBox.getFuiRenderNode()->setY(PANEL_TOP + 60.0f);
    m_doneButton.getFuiRenderNode()->setY(316.0f);
    m_title.UpdateControl();
    m_enableBox.UpdateControl();
    m_doneButton.UpdateControl();
    UIControl_CheckBox* boxes[2] = {&m_miniGameBox, &m_musicBox};
    for (int i = 0; i < 2; i++)
    {
        if (boxes[i]->getFuiRenderNode() == NULL)
            continue;
        boxes[i]->getFuiRenderNode()->setX(m_enableBox.getFuiRenderNode()->getX());
        boxes[i]->getFuiRenderNode()->setY(PANEL_TOP + 92.0f + 32.0f * i);
        boxes[i]->UpdateControl();
    }

    FJ_Base* box = m_enableBox.getFuiRenderNode()->getFuiNode()->asFJ_Base();
    FJ_Base* list = m_packList.getFuiRenderNode()->getFuiNode()->asFJ_Base();
    FJ_Base* done = m_doneButton.getFuiRenderNode()->getFuiNode()->asFJ_Base();
    if (box == NULL || list == NULL || done == NULL)
        return;
    FJ_Base* miniGame =
        m_miniGameBox.getFuiRenderNode() != NULL ? m_miniGameBox.getFuiRenderNode()->getFuiNode()->asFJ_Base() : NULL;
    FJ_Base* music = m_musicBox.getFuiRenderNode() != NULL ? m_musicBox.getFuiRenderNode()->getFuiNode()->asFJ_Base() : NULL;
    if (miniGame != NULL && music != NULL)
    {
        box->PcSetNavUpDown("", "PcMiniGameMusic");
        miniGame->PcSetNavUpDown("CheckboxOnline", "PcDefaultMusic");
        music->PcSetNavUpDown("PcMiniGameMusic", "TexturePackSelector");
        list->PcSetNavUpDown("PcDefaultMusic", "NewWorld");
    }
    else
    {
        box->PcSetNavUpDown("", "TexturePackSelector");
        list->PcSetNavUpDown("CheckboxOnline", "NewWorld");
    }
    done->PcSetNavUpDown("TexturePackSelector", "");
}

void UIScene_PcTextureOverride::tick()
{
    UIScene::tick();
    if (m_movie == NULL || m_movie->getRootNode() == NULL || m_movie->getRootNode()->getFuiNode() == NULL
        || m_left)
        return;
    FJ_FuiNode* focus = m_movie->getRootNode()->getFuiNode()->getStage()->getFocus();
    if (focus != NULL
        && (focus == m_nameBox.getFuiRenderNode()->getFuiNode() || focus == m_modeSlider.getFuiRenderNode()->getFuiNode()
            || focus == m_difficultySlider.getFuiRenderNode()->getFuiNode()
            || focus == m_moreButton.getFuiRenderNode()->getFuiNode()))
        SetFocusToElement(CHECKBOX_ID);
    if (m_pendingSlot >= 0 && m_pendingSlot < (int)m_packs.size())
    {
        ShowPack(m_packs[m_pendingSlot]);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    }
    m_pendingSlot = -1;
}

void UIScene_PcTextureOverride::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_PcTextureOverride::updateComponents()
{
    bool notInGame = Minecraft::GetInstance()->mLevel == NULL;
    m_layer->showComponent(m_playerId, EUIScene_Panorama, notInGame);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_PcTextureOverride::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                             bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            Leave();
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    }
}

void UIScene_PcTextureOverride::handlePress(int controlId, int childId)
{
    if (controlId == LIST_ID && childId >= 0 && childId < (int)m_packs.size())
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        m_slot = childId;
        m_changed = true;
        ShowPack(m_packs[m_slot]);
    }
    if (controlId == DONE_ID)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        Leave();
    }
}

void UIScene_PcTextureOverride::handleCheckboxToggled(int id, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (id == CHECKBOX_ID)
        m_enabled = checked;
    else if (id == MUSIC_ID)
        m_defaultMusic = checked;
    else if (id == MINIGAME_ID)
        m_miniGameMusic = checked;
    else
        return;
    m_changed = true;
}

void UIScene_PcTextureOverride::handleSelectionChanged(int selection)
{
    m_pendingSlot = selection;
}

UIControl* UIScene_PcTextureOverride::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_PcTextureOverride::Leave()
{
    if (m_left)
        return;
    m_left = true;
    if (m_changed)
    {
        PcSettingsSetTextureOverride(m_enabled);
        PcSettingsSetDefaultMusic(m_defaultMusic);
        PcSettingsSetMiniGameMusicOverride(m_miniGameMusic);
        if (m_slot >= 0 && m_slot < (int)m_packs.size())
            PcSettingsSetTextureOverridePack(m_packs[m_slot]);
        PcSettingsSave();
    }
    navigateBack();
}
