#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/UIScene_MapVoteMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_MapVoteMenu::UIScene_MapVoteMenu(int type, void* data, UILayer* layer) : UIScene(type, layer)
{
    m_splitScreen = reinterpret_cast<intptr_t>(data) == 1;
    initialiseMovie();
    m_updateVoteInfo = false;
    m_selectedVote = -1;
    m_previousSelection = -1;
    m_voteTallyText = L"";
    m_votableType = Minecraft::GetInstance()->GetFirstMiniGame()->VotableType();
    Init();
}

UIScene_MapVoteMenu::~UIScene_MapVoteMenu()
{
}

unsigned int UIScene_MapVoteMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_mapList.setupControl(this, currentNode, std::string("MapList"));
        m_controls.push_back(&m_mapList);
    }
    {
        m_mapListTitle.setupControl(this, currentNode, std::string("MapListTitle"));
        m_controls.push_back(&m_mapListTitle);
    }
    {
        m_mapTitle.setupControl(this, currentNode, std::string("MapTitle"));
        m_controls.push_back(&m_mapTitle);
    }
    {
        m_mapDescription.setupControl(this, currentNode, std::string("HTMLMapText"));
        m_controls.push_back(&m_mapDescription);
    }
    {
        m_mapIcon.setupControl(this, currentNode, std::string("MapPic"));
        m_controls.push_back(&m_mapIcon);
    }
    return 1;
}

void UIScene_MapVoteMenu::InitForMap()
{
    LevelRules::ELevelRuleType type = LevelRules::GetRuleTypeForMiniGame(Minecraft::GetMiniGameId());
    gApp.getLevelRules()->getRulesByType(type, &m_levelRuleGroups, false);
    m_mapList.init(0);
    m_mapListTitle.init(UIString(IDS_MG_SELECT_MAP));
    if (m_splitScreen)
        m_mapTitle.init(UIString(L10N::GetString(IDS_MG_MAP_DESC)));
    else
        m_mapTitle.init(UIString(L10N::GetString(IDS_MG_MAP_VOTES)));
    m_mapDescription.init(UIString(L""));

    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    std::vector<ClientMasterGameMode::MapSelectInfo> playlist;
    mode->GetUniquePlaylist(playlist);
    int index = 0;
    std::vector<ClientMasterGameMode::MapSelectInfo>::iterator info = playlist.begin();
    if (info != playlist.end())
    {
        const unsigned int debugArtTools = gApp.DebugArtToolsOn();
        do
        {
            wchar_t textureName[64] = L"default";
            TexturePack* texturePack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(
                info->texturePackId);
            unsigned int iconSize;
            const void* icon = texturePack->getPackIcon(iconSize);
            if (iconSize != 0 && icon != NULL)
            {
                std::swprintf(textureName, 64, L"tpack%08x", texturePack->getId());
                if (!hasRegisteredSubstitutionTexture(textureName))
                    registerSubstitutionTexture(textureName, (unsigned char*)icon, iconSize, false);
            }

            LevelGenerationOptions* options = gApp.getLevelRules()->getLevelGenFromUniqueId(type, info->uniqueId);
            std::wstring name = options != NULL ? options->getWorldName() : L"";
            short mapSize = LevelGenerationOptions::GetMapSizeFromUniqueId(info->uniqueId);
            if (debugArtTools)
            {
                if (mapSize == 0)
                    name = name + L" (Small)";
                else if (mapSize == 2)
                    name = name + L" (Huge)";
            }

            int uniqueId = info->uniqueId;
            bool checked = index == m_selectedVote || mode->GetMapVote(m_playerId, uniqueId);
            if (checked)
                m_selectedVote = index;
            m_mapList.AddNewItem(name, index, checked, textureName);
            m_mapIds.push_back(uniqueId);
            ++index;
            ++info;
        } while (info != playlist.end());
    }
}

void UIScene_MapVoteMenu::InitForVariant()
{
    m_mapList.init(0);
    m_mapListTitle.init(UIString(IDS_MG_SELECT_VARIANT));
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(mode->GetNextGameMode(), true);
    if (m_splitScreen)
        m_mapTitle.init(UIString(L10N::GetString(IDS_MG_VARIANT_DESC)));
    else
        m_mapTitle.init(UIString(L10N::GetString(IDS_MG_VARIANT_VOTES)));
    m_mapDescription.init(UIString(L""));

    int count = definition->GetGameVariantCount() - 2;
    if (definition->IsCustomGame())
        count = definition->GetLootSetCount();
    for (int i = 0; i < count; ++i)
    {
        Minecraft::GetInstance()->GetMiniGame();
        std::wstring iconPath = MiniGameDef::GetVariantIconPath(mode->GetNextGameMode(), i);
        if (gApp.hasArchiveFile(iconPath, false))
        {
            arrayWithLength<unsigned char> file = gApp.getArchiveFile(iconPath, false);
            if (file.length != 0 && file.data != NULL)
                registerSubstitutionTexture(iconPath, file.data, file.length, true);
        }
        std::wstring label = definition->GetLootSetText(i);
        bool checked = i == m_selectedVote || mode->GetMapVote(m_playerId, i);
        if (checked)
            m_selectedVote = i;
        m_mapList.AddNewItem(label, i, checked, iconPath);
        m_mapIds.push_back(i);
    }
}

void UIScene_MapVoteMenu::InitForTheme()
{
    m_mapList.init(0);
    m_mapListTitle.init(UIString(IDS_MG_SELECT_VARIANT));
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (m_splitScreen)
        m_mapTitle.init(UIString(L10N::GetString(IDS_MG_VARIANT_DESC)));
    else
        m_mapTitle.init(UIString(L10N::GetString(IDS_MG_VARIANT_VOTES)));
    m_mapDescription.init(UIString(L""));

    for (unsigned int i = 0; i < mode->mThemeWordIds.size(); ++i)
    {
        Minecraft::GetInstance()->GetMiniGame();
        std::wstring iconPath = MiniGameDef::GetVariantIconPath(mode->GetNextGameMode(), i);
        if (gApp.hasArchiveFile(iconPath, false))
        {
            arrayWithLength<unsigned char> file = gApp.getArchiveFile(iconPath, false);
            if (file.length != 0 && file.data != NULL)
                registerSubstitutionTexture(iconPath, file.data, file.length, true);
        }
        std::wstring label = mode->GetString(mode->mThemeWordIds.at(i));
        bool checked = i == m_selectedVote || mode->GetMapVote(m_playerId, i);
        if (checked)
            m_selectedVote = i;
        m_mapList.AddNewItem(label, i, checked, iconPath);
        m_mapIds.push_back(mode->mThemeWordIds.at(i));
    }
}

void UIScene_MapVoteMenu::Init()
{
    if (m_votableType == 0)
        InitForMap();
    else if (m_votableType == 1)
        InitForVariant();
    else if (m_votableType == 2)
        InitForTheme();
}

void UIScene_MapVoteMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips((unsigned int)m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_MapVoteMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_MapVoteMenu::handleReload()
{
    Init();
}

void UIScene_MapVoteMenu::UpdateVotableInfo()
{
    int id = m_mapIds[m_mapList.getCurrentSelection()];
    if (m_votableType == 0)
    {
        LevelRules::ELevelRuleType type = LevelRules::GetRuleTypeForMiniGame(Minecraft::GetMiniGameId());
        LevelGenerationOptions* options = gApp.getLevelRules()->getLevelGenFromUniqueId(type, id);
        if (options != NULL)
        {
            if (m_splitScreen)
            {
                m_mapTitle.setLabel(UIString(options->getWorldName()), false, false);
                m_mapDescription.setLabel(UIString(options->getWorldDescription()), false, false);
            }
            DLCPack* pack = options->getPack();
            DLCFile* file = pack->getFile(DLCManager::eDLCType_Texture, options->getWorldPreviewImage(), true, true);
            if (file != NULL)
            {
                unsigned int imageSize = 0;
                unsigned char* imageData = file->getData(imageSize);
                if (imageSize != 0 && imageData != NULL)
                {
                    wchar_t textureName[64];
                    std::swprintf(textureName, 64, L"mapSelect%08x", options->getUniqueId());
                    registerSubstitutionTexture(textureName, imageData, imageSize, false);
                    m_mapIcon.setTextureName(textureName, true);
                }
                else
                {
                    m_mapIcon.setTextureName(L"", true);
                }
            }
            else
            {
                m_mapIcon.setTextureName(L"", true);
            }
        }
        else
        {
            m_mapIcon.setTextureName(L"", true);
            if (m_splitScreen)
            {
                m_mapTitle.setLabel(UIString(L""), false, false);
                m_mapDescription.setLabel(UIString(L""), false, false);
            }
        }
    }
    else if (m_votableType == 1)
    {
        MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(
            Minecraft::GetInstance()->GetClientMasterGameMode()->GetNextGameMode(), true);
        if (m_splitScreen)
        {
            m_mapTitle.setLabel(UIString(definition->GetLootSetText(m_mapList.getCurrentSelection())), false, false);
            m_mapDescription.setLabel(UIString(definition->GetLootSetDescription(m_mapList.getCurrentSelection())),
                                      false, false);
            switch (m_mapList.getCurrentSelection())
            {
            case 0:
            {
                m_mapDescription.setLabel(UIString(L10N::GetString(0xea1ca730)), false, false);
            }
            break;
            case 1:
            {
                m_mapDescription.setLabel(UIString(L10N::GetString(0x28850ed6)), false, false);
            }
            break;
            }
        }
    }
    else
    {
        m_mapIcon.setTextureName(L"", true);
        if (m_splitScreen)
        {
            m_mapTitle.setLabel(UIString(L""), false, false);
            m_mapDescription.setLabel(UIString(L""), false, false);
        }
    }
}

void UIScene_MapVoteMenu::tick()
{
    if (m_previousSelection >= 0)
    {
        m_mapList.SetCheckbox(m_previousSelection, false);
        m_previousSelection = -1;
    }
    if (!m_splitScreen)
    {
        std::vector<HtmlString> lines;
        ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
        if (m_votableType == 0)
            mode->GetCurrentMapVotesText(lines);
        else if (m_votableType == 1)
            mode->GetCurrentVariantVotesText(lines);
        else if (m_votableType == 2)
            mode->GetCurrentThemeVotesText(lines);
        std::wstring text = HtmlString::Compose(&lines);
        if (text.compare(m_voteTallyText) != 0)
        {
            m_mapDescription.setLabel(UIString(text), false, false);
            m_voteTallyText = text;
        }
    }
    if (m_updateVoteInfo)
    {
        m_updateVoteInfo = false;
        UpdateVotableInfo();
    }
    UIScene::tick();
}

std::wstring UIScene_MapVoteMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"MapSelectMenuSplit";
    else
        return L"MapSelectMenu";
}

void UIScene_MapVoteMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
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
        break;
    }
}

void UIScene_MapVoteMenu::handlePress(int controlId, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    int previousVote = m_selectedVote;
    if (previousVote != childId)
    {
        m_previousSelection = previousVote;
        m_selectedVote = childId;
        ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
        switch (m_votableType)
        {
        case 0:
        case 1:
            mode->SetMapVote(m_playerId, m_mapIds.begin()[childId], false);
            break;
        case 2:
            mode->SetBuildOffThemeVote(m_playerId, m_mapIds.begin()[childId], 1);
            break;
        }
    }
    else
    {
        m_selectedVote = -1;
        ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
        mode->SetMapVote(m_playerId, m_mapIds.begin()[childId], true);
    }
}

void UIScene_MapVoteMenu::handleFocusChange(int, int childFocus)
{
    m_mapList.updateChildFocus(childFocus);
    m_updateVoteInfo = true;
}

int UIScene_MapVoteMenu::getSceneType()
{
    return EUIScene_MapVoteMenu;
}
