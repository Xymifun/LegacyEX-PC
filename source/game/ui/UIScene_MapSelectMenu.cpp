#include "game/ui/UIScene_MapSelectMenu.hpp"
#include "game/core/StringIds.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"

#include <algorithm>
#include <stdio.h>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_MapSelectMenu::UIScene_MapSelectMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer),
      IUIScene_DlcSelectMenu(eDLCContentType_MapPack),
      m_levelRuleGroups(),
      m_mapList(),
      m_mapListTitle(),
      m_mapTitle(),
      m_mapDescription(),
      m_mapIcon()
{
    initialiseMovie();
    m_initData = static_cast<LaunchMoreOptionsMenuInitData*>(data);
    m_generationOptions = NULL;
    m_playerId = m_initData->mPad;
    LevelRules::ELevelRuleType ruleType = LevelRules::GetRuleTypeForMiniGame(m_initData->mMiniGameId);
    if (m_initData->mMatchmaking && !m_initData->mTimeTrial
        && !Minecraft::GetInstance()->GetMiniGame()->HasFeatureSeasonalMaps())
        gApp.getLevelRules()->getRulesByType(ruleType, &m_initData->mLevelRulesetGroups, true);
    gApp.getLevelRules()->getRulesByType(ruleType, &m_levelRuleGroups, false);
    std::sort(m_levelRuleGroups.begin(), m_levelRuleGroups.end(), LevelRules::LevelRulesetGroup::SortMapRules);
    m_miniGameType = m_initData->mMiniGameId;
    m_mapSelection = 0;
    switch (m_miniGameType)
    {
    case eMiniGameId_Battle:
        m_mapSelection = GameSettings::GetBattleMiniGameMapSelection(m_playerId);
        break;
    case eMiniGameId_Glide:
        m_mapSelection = GameSettings::GetGlideMiniGameMapSelection(m_playerId);
        break;
    }
    _init();
}

UIScene_MapSelectMenu::~UIScene_MapSelectMenu()
{
}

unsigned int UIScene_MapSelectMenu::mapElementsAndNames()
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

void UIScene_MapSelectMenu::_init()
{
    m_mapList.init(0);
    m_mapListTitle.init(UIString(IDS_MG_SELECT_MAPS));
    m_mapTitle.init(UIString(L""));
    m_mapDescription.init(UIString(L""));
    bool anySelected = false;
    int index = 0;
    for (std::vector<LevelRules::LevelRulesetGroup*>::iterator it = m_levelRuleGroups.begin();
         it != m_levelRuleGroups.end(); ++it)
    {
        LevelRules::LevelRulesetGroup* group = *it;
        wchar_t textureName[64] = L"default";

        unsigned int texturePackId = group->rulesets[0]->getGenerationOptions()->getTexturePackId();
        TexturePack* texturePack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(texturePackId);
        if (texturePack == NULL)
            gApp.getDLCFolderName(static_cast<EDLC_ID>(texturePackId));

        unsigned int iconSize;
        const void* iconData = texturePack->getPackIcon(iconSize);
        if (iconSize != 0 && iconData != NULL)
        {
            std::swprintf(textureName, 64, L"tpack%08x", texturePack->getId());
            if (!hasRegisteredSubstitutionTexture(textureName))
                registerSubstitutionTexture(textureName, static_cast<unsigned char*>(const_cast<void*>(iconData)),
                                            iconSize, false);
        }

        LevelGenerationOptions* options = group->rulesets[0]->getGenerationOptions();
        bool selected = (m_mapSelection & (1 << options->getThemeId())) != 0;
        if (!options->getPack()->hasPurchasedPack()
            && !gApp.mDLCManager.IsMiniGameTemporarilyFree(group->rulesets[0]->getRuleType()))
            selected = false;

        if (m_initData->mMatchmaking && !m_initData->mTimeTrial
            && std::find(m_initData->mLevelRulesetGroups.begin(), m_initData->mLevelRulesetGroups.end(), group)
                   != m_initData->mLevelRulesetGroups.end())
        {
            selected = true;
            anySelected = true;
        }
        else if (selected)
        {
            anySelected = true;
        }

        m_mapList.AddNewItem(group->rulesets[0]->getGenerationOptions()->getWorldName(), index, selected, textureName);
        if (m_initData->mMatchmaking && !m_initData->mTimeTrial)
        {
            m_mapList.EnableItem(index, false, true);
            m_mapList.SetCheckbox(index, selected);
        }
        ++index;
    }
    if (!anySelected && m_mapList.getItemCount() >= 4)
    {
        m_mapList.SetCheckbox(m_mapList.getItemCount() - 4, true);
        m_mapList.SetCheckbox(m_mapList.getItemCount() - 3, true);
        m_mapList.SetCheckbox(m_mapList.getItemCount() - 2, true);
        m_mapList.SetCheckbox(m_mapList.getItemCount() - 1, true);
    }
    gConsoleUIController.TouchBoxRebuild(this);
    m_mapInfoPending = 1;
}

std::wstring UIScene_MapSelectMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"MapSelectMenuSplit";
    return L"MapSelectMenu";
}

void UIScene_MapSelectMenu::updateTooltips()
{
    if (m_initData->mMatchmaking && !m_initData->mTimeTrial)
        gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, false, false);
    else
        gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_SELECT_ALL,
                                         IDS_TOOLTIPS_UNSELECT_ALL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         false, false);
}

void UIScene_MapSelectMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_MapSelectMenu::UpdateMapInfo()
{
    LevelGenerationOptions* options =
        m_levelRuleGroups.at(m_mapList.getCurrentSelection())->rulesets.at(0)->getGenerationOptions();
    m_mapTitle.setLabel(UIString(options->getWorldName()), false, false);
    m_mapDescription.setLabel(UIString(options->getWorldDescription()), false, false);

    DLCPack* pack = options->getPack();
    DLCFile* file = pack->getFile(DLCManager::eDLCType_Texture, std::wstring(options->getWorldPreviewImage()), true,
                                  true);
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
}

void UIScene_MapSelectMenu::tick()
{
    UIScene::tick();
    if (m_mapInfoPending != 0)
    {
        m_mapInfoPending = 0;
        UpdateMapInfo();
    }
    IUIScene_DlcSelectMenu::tick();
}

void UIScene_MapSelectMenu::handleReload()
{
    _init();
}

int UIScene_MapSelectMenu::checkHasPurchased(int index, bool tryPurchase)
{
    LevelRules::LevelRulesetGroup* group = m_levelRuleGroups.at(index);
    LevelRuleset* ruleset = group->rulesets.at(0);
    LevelGenerationOptions* options = ruleset->getGenerationOptions();
    DLCPack* pack = options->getPack();
    bool purchased = pack->hasPurchasedPack() || gApp.mDLCManager.IsMiniGameTemporarilyFree(ruleset->getRuleType());
    if (!purchased && tryPurchase)
    {
        bool handled = false;
        cDLCInfo* dlcInfo = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance());
        DLC_INFO* info = dlcInfo->GetDLCInfoForPackID(pack->GetPackId());
        int contentType = info == NULL ? 0 : info->ContentType;
        tryPurchaseDlc(m_playerId, handled, (eDLCContentType)contentType);
    }
    return purchased;
}

void UIScene_MapSelectMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_purchasePending)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
    {
        if (!pressed)
            return;
        unsigned int selection = 0;
        m_initData->mLevelRulesetGroups.clear();
        for (int i = 0; i < m_mapList.getItemCount(); ++i)
        {
            if (m_mapList.GetCheckbox(i))
            {
                selection |= 1 << m_levelRuleGroups.at(i)->rulesets[0]->getGenerationOptions()->getThemeId();
                m_initData->mLevelRulesetGroups.push_back(m_levelRuleGroups.at(i));
            }
        }
        if (m_initData->mLevelRulesetGroups.size() != 0)
        {
            if (selection != m_mapSelection)
            {
                switch (m_miniGameType)
                {
                case eMiniGameId_Battle:
                    GameSettings::SetBattleMiniGameMapSelection(m_playerId, selection);
                    m_mapSelection = selection;
                    break;
                case eMiniGameId_Glide:
                    GameSettings::SetGlideMiniGameMapSelection(m_playerId, selection);
                    m_mapSelection = selection;
                    break;
                default:
                    m_mapSelection = selection;
                    break;
                }
            }
            GameSettings::CheckGameSettingsChanged(true, m_playerId);
            navigateBack();
        }
        else
        {
            int parameters[1] = {IDS_CONFIRM_OK};
            m_mapSelection = 0;
            gConsoleUIController.RequestAlertMessage(IDS_MG_NO_MAPS_SELECTED_TITLE, IDS_MG_NO_MAPS_SELECTED_TEXT,
                                                     parameters, 1, m_playerId, NULL, NULL, NULL, true);
        }
        handled = true;
        break;
    }
    case EControllerActions_MenuOk:
        if (!pressed || repeat)
            return;
        if (checkHasPurchased((int)m_mapList.getCurrentSelection(), true) == 0)
            return;
        sendInputToMovie(action, false, pressed, released);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case EControllerActions_MenuX:
        if (m_initData->mMatchmaking && !m_initData->mTimeTrial)
            return;
        if (!pressed || repeat)
            return;
        for (int i = 0; i < m_mapList.getItemCount(); ++i)
            if (checkHasPurchased(i, false) != 0)
                m_mapList.SetCheckbox(i, true);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case EControllerActions_MenuY:
        if (m_initData->mMatchmaking && !m_initData->mTimeTrial)
            return;
        if (!pressed || repeat)
            return;
        for (int i = 0; i < m_mapList.getItemCount(); ++i)
            if (checkHasPurchased(i, false) != 0)
                m_mapList.SetCheckbox(i, false);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_MapSelectMenu::handleFocusChange(int, int childFocus)
{
    m_mapList.updateChildFocus(childFocus);
    m_mapInfoPending = 1;
    m_generationOptions = m_levelRuleGroups.at(m_mapList.getCurrentSelection())->rulesets.at(0)->getGenerationOptions();
    m_pack = m_generationOptions->getPack();
}

void UIScene_MapSelectMenu::handlePress(int, int)
{
}

int UIScene_MapSelectMenu::getSceneType()
{
    return EUIScene_MapSelectMenu;
}
