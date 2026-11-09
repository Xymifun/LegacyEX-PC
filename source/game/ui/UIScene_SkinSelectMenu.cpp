#include "game/ui/UIScene_SkinSelectMenu.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <cstdio>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringConversion.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_SkinSelect.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"

extern CConsoleMinecraftApp gApp;
extern ConsoleUIController gConsoleUIController;

static const int s_defaultSkinNameIds[19] = {-1,
                                             IDS_DEFAULT_SKIN0,
                                             IDS_DEFAULT_SKIN1,
                                             IDS_DEFAULT_SKIN2,
                                             IDS_DEFAULT_SKIN3,
                                             IDS_DEFAULT_SKIN4,
                                             IDS_DEFAULT_SKIN5,
                                             IDS_DEFAULT_SKIN6,
                                             IDS_DEFAULT_SKIN7,
                                             IDS_DEFAULT_SKIN8,
                                             IDS_DEFAULT_SKIN9,
                                             IDS_DEFAULT_SKIN10,
                                             IDS_DEFAULT_SKIN11,
                                             IDS_DEFAULT_SKIN12,
                                             IDS_DEFAULT_SKIN13,
                                             IDS_DEFAULT_SKIN14,
                                             IDS_DEFAULT_SKIN15,
                                             IDS_DEFAULT_SKIN17,
                                             IDS_DEFAULT_SKIN16};

static const unsigned int s_dlcSkinIdMask = 0x7fffffff;

static int s_newestSkinPackIds[12] = {0x22c, 0x224, 0x233, 0x222, 0x216, 0x225,
                                      0x234, 0x207, 0x227, 0x4f9, 0x23c, 0x40e};

UIScene_SkinSelectMenu::UIScene_SkinSelectMenu(int player, void* data, UILayer* layer)
    : UIScene(player, layer),
      IUIScene_DlcSelectMenu(eDLCContentType_SkinPack)
{
    initialiseMovie();
    m_packList.init(0);

    m_noSkinsToShow = false;
    m_skinIndex = 0;
    m_packIndex = 0;
    m_selectedPackId = 0;
    m_originalSkinId = static_cast<int>(GameSettings::GetPlayerSkinId(player));
    m_currentSkinPath = GameSettings::GetPlayerSkinName(player);
    m_selectedSkinPath = L"";
    m_selectedCapePath = L"";
    m_additionalSkinBoxes = NULL;
    m_texturePackSkinPackId = 0;
    m_skinSliding = false;
    m_hasTexturePackSkinPack = false;
    m_skinAnimating = false;
    m_packFocusChanged = false;
    m_packListDirty = true;
    m_skinUpdatePending = false;

    m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, false);
    m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, false);
    m_characters[eCharacter_Next2].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, false);
    m_characters[eCharacter_Next3].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, false);
    m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, false);
    m_characters[eCharacter_Previous2].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, false);
    m_characters[eCharacter_Previous3].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, false);

    m_labelSkinName.init(UIString(L""));
    m_labelSkinOrigin.init(UIString(L""));
    m_labelPackName.init(UIString(L""));
    m_labelPackType.init(UIString(L""));
    m_touchCharacters.init(10);

    if (gApp.StartInstallDLCProcess(m_playerId) != 0)
    {
        m_purchasePending = true;
        m_controlTimer.setVisible(true);
        m_controlCharacters.setVisible(false);
        setCharacterLocked(false);
        setCharacterSelected(false);
        setCharacterFavourite(false);
        setCharacterBlocked(false);
    }
    else
    {
        m_controlTimer.setVisible(false);
        GameSettings::ValidateFavoriteSkins(m_playerId);
        setActivePackIndex();
        handlePackIndexChanged();
    }
}

UIScene_SkinSelectMenu::~UIScene_SkinSelectMenu()
{
}

unsigned int UIScene_SkinSelectMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_touchCharacters.setupControl(this, currentNode, std::string("TouchIggyCharacters"));
        m_controls.push_back(&m_touchCharacters);
    }
    {
        m_labelPackName.setupControl(this, currentNode, std::string("Pack_Name"));
        m_controls.push_back(&m_labelPackName);
    }
    {
        m_labelPackType.setupControl(this, currentNode, std::string("Pack_Type"));
        m_controls.push_back(&m_labelPackType);
    }
    {
        m_labelSkinName.setupControl(this, currentNode, std::string("SkinTitle1"));
        m_controls.push_back(&m_labelSkinName);
    }
    {
        m_labelSkinOrigin.setupControl(this, currentNode, std::string("SkinTitle2"));
        m_controls.push_back(&m_labelSkinOrigin);
    }
    {
        m_controlTimer.setupControl(this, currentNode, std::string("Timer"));
        m_controls.push_back(&m_controlTimer);
    }
    {
        m_controlCharacters.setupControl(this, currentNode, std::string("IggyCharacters"));
        m_controls.push_back(&m_controlCharacters);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_controlCharacters.getFuiRenderNode();
        {
            m_characters[0].setupControl(this, currentNode, std::string("iggy_Character0"));
            m_controls.push_back(&m_characters[0]);
        }
        {
            m_characters[1].setupControl(this, currentNode, std::string("iggy_Character1"));
            m_controls.push_back(&m_characters[1]);
        }
        {
            m_characters[2].setupControl(this, currentNode, std::string("iggy_Character2"));
            m_controls.push_back(&m_characters[2]);
        }
        {
            m_characters[3].setupControl(this, currentNode, std::string("iggy_Character3"));
            m_controls.push_back(&m_characters[3]);
        }
        {
            m_characters[4].setupControl(this, currentNode, std::string("iggy_Character4"));
            m_controls.push_back(&m_characters[4]);
        }
        {
            m_characters[5].setupControl(this, currentNode, std::string("iggy_Character5"));
            m_controls.push_back(&m_characters[5]);
        }
        {
            m_characters[6].setupControl(this, currentNode, std::string("iggy_Character6"));
            m_controls.push_back(&m_characters[6]);
        }
        currentNode = parentNode;
    }
    {
        m_packIcon.setupControl(this, currentNode, std::string("TexturePackIcon"));
        m_controls.push_back(&m_packIcon);
    }
    {
        m_packList.setupControl(this, currentNode, std::string("SkinButtonList"));
        m_controls.push_back(&m_packList);
    }
    return 1;
}

void UIScene_SkinSelectMenu::setCharacterLocked(bool locked)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetCharacterLocked(locked);
}

void UIScene_SkinSelectMenu::setCharacterSelected(bool selected)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetPlayerCharacterSelected(selected, false);
}

void UIScene_SkinSelectMenu::setCharacterFavourite(bool favourite)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetCharacterFavourite(favourite);
}

void UIScene_SkinSelectMenu::setCharacterBlocked(bool blocked)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetCharacterBlocked(blocked);
}

void UIScene_SkinSelectMenu::setActivePackIndex()
{
    if (gApp.mDLCManager.getPackCount(DLCManager::eDLCType_Skin) == 0)
        return;

    m_pack = gApp.mDLCManager.getPackContainingSkin(m_currentSkinPath);
    if (m_pack != NULL)
        m_selectedPackId = static_cast<int>(m_pack->GetPackId());

    bool isDefaultSkin = false;
    bool hasPurchasedPack = false;
    const unsigned int favouriteCount = GameSettings::GetPlayerFavoriteSkinsCount(m_playerId);
    for (unsigned int i = 0; i < favouriteCount; ++i)
    {
        DLCFile* skinFile = gApp.mDLCManager.getSkinFile(
            static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(i))), isDefaultSkin,
            hasPurchasedPack);
        if (!isDefaultSkin)
        {
            if (skinFile != NULL)
            {
                std::wstring skinPath = skinFile->getPath();
                if (skinPath.compare(m_currentSkinPath) == 0)
                {
                    m_packIndex = 1;
                    m_selectedPackId = 0;
                    return;
                }
            }
        }
        else if (m_originalSkinId
                 == static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(i))))
        {
            m_selectedPackId = 0;
            m_packIndex = 1;
            return;
        }
    }
}

unsigned int UIScene_SkinSelectMenu::registerTexture(std::wstring textureName)
{
    wchar_t path[64];
    swprintf(path, 64, L"Graphics\\PackGraphics\\%ls.png", textureName.c_str());
    if (gApp.hasArchiveFile(std::wstring(path), false))
    {
        arrayWithLength<unsigned char> file = gApp.getArchiveFile(std::wstring(path), false);
        if (file.length != 0 && file.data != NULL)
            registerSubstitutionTexture(textureName, file.data, file.length, true);
        return 1;
    }
    return 0;
}

unsigned char UIScene_SkinSelectMenu::getTextureId(int index)
{
    static const unsigned char textureIds[19] = {0x25, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x70,
                                                 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0xaa, 0xa9};
    if (static_cast<unsigned int>(index) > 0x12)
        return 0x25;
    return textureIds[index];
}

unsigned int UIScene_SkinSelectMenu::getNextSkinIndex(unsigned int index)
{
    unsigned int next = index + 1;
    if (m_packIndex == 1)
    {
        if (next >= GameSettings::GetPlayerFavoriteSkinsCount(m_playerId))
            next = 0;
    }
    else if (m_packIndex == 0 && static_cast<int>(next) >= 0x13)
    {
        next = 0;
    }
    else if (m_pack != NULL)
    {
        if (next >= static_cast<unsigned int>(m_pack->getSkinCount()))
            next = 0;
    }
    return next;
}

int UIScene_SkinSelectMenu::getPreviousSkinIndex(unsigned int index)
{
    unsigned int previous = index;
    if (previous == 0)
    {
        if (m_packIndex == 1)
        {
            previous = GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) - 1;
        }
        else if (m_packIndex == 0)
        {
            previous = 0x12;
        }
        else
        {
            if (m_pack != NULL)
                previous = m_pack->getSkinCount() - 1;
        }
    }
    else
    {
        previous = previous - 1;
    }
    return previous;
}

void UIScene_SkinSelectMenu::handleSkinIndexChanged()
{
    std::wstring skinName = L"";
    std::wstring skinOrigin = L"";
    DLCSkinFile* skinFile = NULL;
    _TEXTURE_NAME backupTexture = TN_MOB_STEVE;

    m_noSkinsToShow = false;
    setCharacterSelected(false);

    if (m_pack != NULL)
    {
        skinFile = static_cast<DLCSkinFile*>(m_pack->getSkinFile(m_skinIndex));
        m_selectedSkinPath = skinFile->getPath();
        m_selectedCapePath = skinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath, L"");
        m_additionalSkinBoxes = skinFile->getAdditionalBoxes();
        skinName = skinFile->getParameterAsString(DLCManager::eDLCParameterType_DisplayName, L"");
        skinOrigin = skinFile->getParameterAsString(DLCManager::eDLCParameterType_ThemeName, L"");

        bool smallSkinBlocked = false;
        if (Minecraft::GetInstance()->GetMiniGame()->AllowSmallSkins()
            && (skinFile->getSkinAdjustments().mGameFlags & 1) != 0)
            smallSkinBlocked = true;
        const bool skinIsFree = skinFile->getParameterAsBool(DLCManager::eDLCParameterType_Free);
        const bool licensed = m_pack->hasPurchasedPack();
        const bool available = skinIsFree || licensed;

        if (m_selectedSkinPath.compare(m_currentSkinPath) == 0 && available)
            setCharacterSelected(!smallSkinBlocked);
        setCharacterLocked(!available);
        setCharacterBlocked(available && smallSkinBlocked);
        m_characters[eCharacter_Current].setVisible(true);
    }
    else
    {
        m_selectedSkinPath = L"";
        m_selectedCapePath = L"";
        m_additionalSkinBoxes = NULL;

        switch (m_packIndex)
        {
        case 0:
            backupTexture = static_cast<_TEXTURE_NAME>(getTextureId(static_cast<int>(m_skinIndex)));
            if (m_skinIndex == 0)
                skinName = L10N::GetString(IDS_DEFAULT_SKINS);
            else
                skinName = L10N::GetString(s_defaultSkinNameIds[m_skinIndex]);
            if (m_originalSkinId == static_cast<int>(m_skinIndex))
                setCharacterSelected(true);
            setCharacterLocked(false);
            setCharacterBlocked(false);
            m_characters[eCharacter_Current].setVisible(true);
            break;
        case 1:
            if (GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) > 0)
            {
                bool isDefaultSkin = false;
                bool licensed = false;
                skinFile = static_cast<DLCSkinFile*>(gApp.mDLCManager.getSkinFile(
                    static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(m_skinIndex))),
                    isDefaultSkin, licensed));
                if (isDefaultSkin)
                {
                    const int favouriteSkin = static_cast<int>(
                        GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(m_skinIndex)));
                    backupTexture = static_cast<_TEXTURE_NAME>(getTextureId(favouriteSkin));
                    if (favouriteSkin == 0)
                        skinName = L10N::GetString(IDS_DEFAULT_SKINS);
                    else
                        skinName = L10N::GetString(s_defaultSkinNameIds[favouriteSkin]);
                    if (m_originalSkinId
                        == static_cast<int>(
                            GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(m_skinIndex))))
                        setCharacterSelected(true);
                    else
                        setCharacterSelected(false);
                    setCharacterLocked(false);
                    setCharacterBlocked(false);
                    m_characters[eCharacter_Current].setVisible(true);
                }
                else if (skinFile != NULL)
                {
                    m_selectedSkinPath = skinFile->getPath();
                    m_selectedCapePath = skinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath, L"");
                    m_additionalSkinBoxes = skinFile->getAdditionalBoxes();
                    skinName = skinFile->getParameterAsString(DLCManager::eDLCParameterType_DisplayName, L"");
                    skinOrigin = skinFile->getParameterAsString(DLCManager::eDLCParameterType_ThemeName, L"");

                    bool smallSkinBlocked = false;
                    if (Minecraft::GetInstance()->GetMiniGame()->AllowSmallSkins()
                        && (skinFile->getSkinAdjustments().mGameFlags & 1) != 0)
                        smallSkinBlocked = true;
                    const bool skinIsFree = skinFile->getParameterAsBool(DLCManager::eDLCParameterType_Free);
                    const bool available = skinIsFree || licensed;

                    if (m_selectedSkinPath.compare(m_currentSkinPath) == 0 && available)
                        setCharacterSelected(!smallSkinBlocked);
                    setCharacterLocked(!available);
                    setCharacterBlocked(available && smallSkinBlocked);
                }
                else
                {
                    setCharacterSelected(false);
                    setCharacterLocked(false);
                    setCharacterBlocked(false);
                }
            }
            else
            {
                m_characters[eCharacter_Current].setVisible(false);
                m_noSkinsToShow = true;
            }
            break;
        }
    }

    if (m_packIndex == 1)
    {
        if (GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) != 0)
            setCharacterFavourite(true);
    }
    else
    {
        const unsigned int favouriteCount = GameSettings::GetPlayerFavoriteSkinsCount(m_playerId);
        setCharacterFavourite(false);
        for (unsigned int i = 0; i < favouriteCount; ++i)
        {
            const int favouriteSkin = static_cast<int>(
                GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(i)));
            int skinId;
            if (m_pack == NULL)
                skinId = static_cast<int>(m_skinIndex);
            else
                skinId = static_cast<int>(m_pack->getSkinFile(m_skinIndex)->m_fileID);
            if (favouriteSkin == skinId)
            {
                setCharacterFavourite(true);
                break;
            }
        }
    }

    m_labelSkinName.setLabel(UIString(skinName), false, false);
    m_labelSkinOrigin.setLabel(UIString(skinOrigin), false, false);

    if (m_additionalSkinBoxes != NULL && m_additionalSkinBoxes->size() != 0
        && gApp.GetAdditionalModelParts(skinFile->m_fileID) == NULL)
    {
        gApp.SetAdditionalSkinBoxes(skinFile->m_fileID, m_additionalSkinBoxes, skinFile->getSkinAdjustments());
    }
    if (skinFile != NULL)
        gApp.SetSkinAdjustments(skinFile->m_fileID, skinFile->getSkinAdjustments());

    m_characters[eCharacter_Current].SetTexture(m_selectedSkinPath, backupTexture);
    m_characters[eCharacter_Current].mCapePath = m_selectedCapePath;

    unsigned int nextIndex = getNextSkinIndex(m_skinIndex);
    unsigned int previousIndex = static_cast<unsigned int>(getPreviousSkinIndex(m_skinIndex));
    std::wstring otherSkinPath = L"";
    std::wstring otherCapePath = L"";

    for (int i = eCharacter_Next1; i < eCharacter_Count; ++i)
        m_characters[i].setVisible(false);

    const unsigned int favouriteCount = GameSettings::GetPlayerFavoriteSkinsCount(m_playerId);
    unsigned char nextCount;
    unsigned char previousCount;
    if (m_packIndex == 1 && favouriteCount < 7)
    {
        if (favouriteCount == 0)
        {
            nextCount = 0;
            previousCount = 0;
        }
        else if (((favouriteCount - 1) & 1) == 0)
        {
            nextCount = static_cast<unsigned char>((favouriteCount - 1) / 2);
            previousCount = nextCount;
        }
        else
        {
            nextCount = static_cast<unsigned char>((favouriteCount - 1) / 2);
            previousCount = static_cast<unsigned char>((favouriteCount - 1) / 2 + 1);
        }
    }
    else
    {
        nextCount = 3;
        previousCount = 3;
    }

    for (unsigned char i = 0; i < nextCount; ++i)
    {
        UIControl_PlayerSkinPreview& preview = m_characters[eCharacter_Next1 + i];
        DLCSkinFile* otherSkinFile = NULL;
        std::vector<SKIN_BOX*>* otherSkinBoxes = NULL;

        preview.setVisible(true);
        if (m_pack != NULL)
        {
            otherSkinFile = static_cast<DLCSkinFile*>(m_pack->getSkinFile(nextIndex));
            otherSkinPath = otherSkinFile->getPath();
            otherCapePath = otherSkinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath, L"");
            otherSkinBoxes = otherSkinFile->getAdditionalBoxes();
            backupTexture = TN_MOB_STEVE;
        }
        else
        {
            otherSkinPath = L"";
            otherCapePath = L"";
            switch (m_packIndex)
            {
            case 0:
                backupTexture = static_cast<_TEXTURE_NAME>(getTextureId(static_cast<int>(nextIndex)));
                break;
            case 1:
            {
                bool isDefaultSkin = false;
                bool licensed = false;
                otherSkinFile = static_cast<DLCSkinFile*>(gApp.mDLCManager.getSkinFile(
                    static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(nextIndex))),
                    isDefaultSkin, licensed));
                if (!isDefaultSkin)
                {
                    if (otherSkinFile != NULL)
                    {
                        otherSkinPath = otherSkinFile->getPath();
                        otherCapePath = otherSkinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath,
                                                                            L"");
                        otherSkinBoxes = otherSkinFile->getAdditionalBoxes();
                        backupTexture = TN_MOB_STEVE;
                    }
                }
                else
                {
                    backupTexture = static_cast<_TEXTURE_NAME>(getTextureId(static_cast<int>(
                        GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(nextIndex)))));
                }
                break;
            }
            }
        }

        if (otherSkinFile != NULL)
        {
            if (otherSkinBoxes != NULL && otherSkinBoxes->size() != 0
                && gApp.GetAdditionalModelParts(otherSkinFile->m_fileID) == NULL)
            {
                gApp.SetAdditionalSkinBoxes(otherSkinFile->m_fileID, otherSkinBoxes,
                                            otherSkinFile->getSkinAdjustments());
            }
            gApp.SetSkinAdjustments(otherSkinFile->m_fileID, otherSkinFile->getSkinAdjustments());
        }

        preview.SetTexture(otherSkinPath, backupTexture);
        preview.mCapePath = otherCapePath;
        nextIndex = getNextSkinIndex(nextIndex);
    }

    for (unsigned char i = 0; i < previousCount; ++i)
    {
        UIControl_PlayerSkinPreview& preview = m_characters[eCharacter_Previous1 + i];
        DLCSkinFile* otherSkinFile = NULL;
        std::vector<SKIN_BOX*>* otherSkinBoxes = NULL;

        preview.setVisible(true);
        if (m_pack != NULL)
        {
            otherSkinFile = static_cast<DLCSkinFile*>(m_pack->getSkinFile(previousIndex));
            otherSkinPath = otherSkinFile->getPath();
            otherCapePath = otherSkinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath, L"");
            otherSkinBoxes = otherSkinFile->getAdditionalBoxes();
            backupTexture = TN_MOB_STEVE;
        }
        else
        {
            otherSkinPath = L"";
            otherCapePath = L"";
            switch (m_packIndex)
            {
            case 0:
                backupTexture = static_cast<_TEXTURE_NAME>(getTextureId(static_cast<int>(previousIndex)));
                break;
            case 1:
            {
                bool isDefaultSkin = false;
                bool licensed = false;
                otherSkinFile = static_cast<DLCSkinFile*>(gApp.mDLCManager.getSkinFile(
                    static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(previousIndex))),
                    isDefaultSkin, licensed));
                if (!isDefaultSkin)
                {
                    if (otherSkinFile != NULL)
                    {
                        otherSkinPath = otherSkinFile->getPath();
                        otherCapePath = otherSkinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath,
                                                                            L"");
                        otherSkinBoxes = otherSkinFile->getAdditionalBoxes();
                        backupTexture = TN_MOB_STEVE;
                    }
                }
                else
                {
                    backupTexture = static_cast<_TEXTURE_NAME>(getTextureId(static_cast<int>(
                        GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(previousIndex)))));
                }
                break;
            }
            }
        }

        if (otherSkinFile != NULL)
        {
            if (otherSkinBoxes != NULL && otherSkinBoxes->size() != 0
                && gApp.GetAdditionalModelParts(otherSkinFile->m_fileID) == NULL)
            {
                gApp.SetAdditionalSkinBoxes(otherSkinFile->m_fileID, otherSkinBoxes,
                                            otherSkinFile->getSkinAdjustments());
            }
            gApp.SetSkinAdjustments(otherSkinFile->m_fileID, otherSkinFile->getSkinAdjustments());
        }

        preview.SetTexture(otherSkinPath, backupTexture);
        preview.mCapePath = otherCapePath;
        previousIndex = static_cast<unsigned int>(getPreviousSkinIndex(previousIndex));
    }

    updateTooltips();
}

unsigned char UIScene_SkinSelectMenu::IsTrialDiscontinuedPack(int packId)
{
    if (packId >= 2)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(packId);
        DLC_INFO* info = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(static_cast<unsigned int>(packId));
        if (info != NULL && info->PackType == 4
            && GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(2), -1) == 0)
        {
            return pack != NULL && !pack->hasPurchasedPack();
        }
    }
    return 0;
}

unsigned int UIScene_SkinSelectMenu::GetIsNewPack(int packId)
{
    for (int i = 0; i < 12; ++i)
    {
        if (s_newestSkinPackIds[i] == packId)
            return 1;
    }
    return 0;
}

bool UIScene_SkinSelectMenu::SORTNEWEST(int first, int second)
{
    int firstPos = 0;
    int secondPos = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (s_newestSkinPackIds[i] == first)
            firstPos = i;
        if (s_newestSkinPackIds[i] == second)
            secondPos = i;
    }
    return firstPos < secondPos;
}

bool UIScene_SkinSelectMenu::SORTALPHABETICALLY(int first, int second)
{
    DLCPack* firstPack = gApp.mDLCManager.getPackFromPackID(first);
    DLCPack* secondPack = gApp.mDLCManager.getPackFromPackID(second);
    if (firstPack != NULL && secondPack != NULL)
        return firstPack->getLocalisedDisplayName().compare(secondPack->getLocalisedDisplayName()) < 0;
    return first > second;
}

void UIScene_SkinSelectMenu::SetSkinPackButtonList()
{
    if (!m_packListDirty)
        return;
    m_packListDirty = false;

    DLCPack* texturePackSkinPack = NULL;
    unsigned int packIndex = m_packIndex;
    const bool usingDefaultPack = Minecraft::GetInstance()->mTexturePackRepository->getSelected()
                                  == TexturePackRepository::defaultTexturePack;
    if (!usingDefaultPack)
    {
        texturePackSkinPack = static_cast<DLCTexturePack*>(
                                  Minecraft::GetInstance()->mTexturePackRepository->getSelected())
                                  ->getDLCInfoParentPack();
    }

    m_packList.clearList();
    m_packList.AddNewButton(L10N::GetString(IDS_NO_SKIN_PACK), 0, true);
    m_packList.AddNewButton(L10N::GetString(IDS_FAVORITES_SKIN_PACK), 1, true);

    if (texturePackSkinPack != NULL && texturePackSkinPack->GetPackId() >= 0x400)
    {
        std::wstring texturePackName = texturePackSkinPack->getLocalisedDisplayName();
        if (texturePackSkinPack->getSkinCount() != 0)
            m_hasTexturePackSkinPack = true;
    }

    m_packIds.clear();
    m_newPackIds.clear();
    m_addedPackIds.clear();

    for (unsigned int i = 0; i < static_cast<unsigned int>(gApp.mDLCManager.getPackCount(DLCManager::eDLCType_Skin));
         ++i)
    {
        DLCPack* pack = gApp.mDLCManager.getPack(i, DLCManager::eDLCType_Skin);
        if (pack == NULL)
            continue;

        DLC_INFO* info = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(pack->GetPackId());
        if (!pack->hasPurchasedPack() && info != NULL && info->PackType == 5)
            continue;

        if (IsTrialDiscontinuedPack(static_cast<int>(pack->GetPackId())) == 0)
        {
            if (m_hasTexturePackSkinPack && texturePackSkinPack != NULL && texturePackSkinPack->GetPackId() >= 0x400
                && texturePackSkinPack->GetPackId() == pack->GetPackId())
            {
                m_texturePackSkinPackId = static_cast<int>(texturePackSkinPack->GetPackId());
            }
            if (GetIsNewPack(static_cast<int>(pack->GetPackId())) != 0)
                m_newPackIds.push_back(static_cast<int>(pack->GetPackId()));
            m_packIds.push_back(static_cast<int>(pack->GetPackId()));
        }

        if (m_selectedPackId != 0 && m_selectedPackId == static_cast<int>(pack->GetPackId()))
        {
            packIndex = pack->GetPackId();
            if (!m_hasTexturePackSkinPack || packIndex != texturePackSkinPack->GetPackId())
            {
                std::wstring packName = pack->getLocalisedDisplayName();
                m_packList.AddNewButton(packName.c_str(), static_cast<int>(pack->GetPackId()), true);
                m_addedPackIds.push_back(static_cast<int>(pack->GetPackId()));
            }
        }
    }

    if (m_hasTexturePackSkinPack)
    {
        std::wstring packName = texturePackSkinPack->getLocalisedDisplayName();
        m_packList.AddNewButton(packName.c_str(), static_cast<int>(texturePackSkinPack->GetPackId()), true);
        m_addedPackIds.push_back(static_cast<int>(texturePackSkinPack->GetPackId()));
    }

    std::sort(m_newPackIds.begin(), m_newPackIds.end(), SORTNEWEST);
    std::sort(m_packIds.begin(), m_packIds.end(), SORTALPHABETICALLY);

#ifdef MINECRAFT_PC
    const int checkCount = 0x7fffffff;
#else
    const int checkCount = 6;
#endif
    int addedChecksRemaining = checkCount;
    int newChecksRemaining = checkCount;
    for (unsigned int i = 0; i < m_newPackIds.size(); ++i)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_newPackIds.at(i));
        if (pack == NULL)
            continue;
        if (addedChecksRemaining > 0)
        {
            if (std::find(m_addedPackIds.begin(), m_addedPackIds.end(), m_newPackIds.at(i)) != m_addedPackIds.end())
                continue;
            --addedChecksRemaining;
        }
        m_packList.AddNewButton(pack->getLocalisedDisplayName().c_str(), m_newPackIds.at(i), true);
    }

    for (unsigned int i = 0; i < m_packIds.size(); ++i)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_packIds.at(i));
        if (pack == NULL)
            continue;
        if (addedChecksRemaining > 0)
        {
            if (std::find(m_addedPackIds.begin(), m_addedPackIds.end(), m_packIds.at(i)) != m_addedPackIds.end())
                continue;
            --addedChecksRemaining;
        }
        if (newChecksRemaining > 0)
        {
            if (std::find(m_newPackIds.begin(), m_newPackIds.end(), m_packIds.at(i)) != m_newPackIds.end())
                continue;
            --newChecksRemaining;
        }
        m_packList.AddNewButton(pack->getLocalisedDisplayName().c_str(), m_packIds.at(i), true);
    }

    m_packIndex = packIndex;
    if (m_hasTexturePackSkinPack)
        m_packList.HighlightItem(static_cast<unsigned int>(m_texturePackSkinPackId), false);
    else
        m_packList.HighlightItem(packIndex, false);
}

void UIScene_SkinSelectMenu::setPackLabel()
{
    std::wstring packName = L"";
    if (m_packIndex < 2)
    {
        if (m_packIndex == 1)
            packName = L10N::GetString(IDS_FAVORITES_SKIN_PACK);
        else
            packName = L10N::GetString(IDS_NO_SKIN_PACK);
    }
    else
    {
        packName = gApp.mDLCManager.getPackFromPackID(static_cast<int>(m_packIndex))->getLocalisedDisplayName();
    }
    m_labelPackName.setLabel(UIString(packName), false, false);

    std::wstring packType = L"";
    if (m_packIndex > 1)
    {
        if (m_packIndex - 0x400 < 0x400)
            packType = L10N::GetString(IDS_MASHUP_PACK);
        else
            packType = L10N::GetString(IDS_SKIN_PACK);
    }
    m_labelPackType.setLabel(UIString(packType), false, false);
}

void UIScene_SkinSelectMenu::updatePackDisplay()
{
    setPackLabel();
}

void UIScene_SkinSelectMenu::handlePackIndexChanged()
{
    if (!m_packListDirty && static_cast<int>(m_packList.getItemCount()) > 0)
        m_packIndex = m_packList.getCurrentSelection();

    DLCPack* pack = NULL;
    if (m_packIndex > 1)
        pack = gApp.mDLCManager.getPackFromPackID(static_cast<int>(m_packIndex));
    m_skinIndex = 0;
    m_pack = pack;

    if (pack != NULL)
    {
        bool found = false;
        const int skinIndex = pack->getSkinIndexAt(m_currentSkinPath, found);
        if (found)
            m_skinIndex = static_cast<unsigned int>(skinIndex);

        if (gApp.GetLocalPlayerCount() < 2 && gConsoleUIController.m_screenHeight != 480.0f)
        {
            const int packId = static_cast<int>(m_pack->GetPackId());
            if (packId < 1)
            {
                registerTexture(L"default");
                m_packIcon.setTextureName(L"default", true);
            }
            else if (registerTexture(_toString<int>(packId)) == 0)
            {
                registerTexture(L"default");
                m_packIcon.setTextureName(L"default", true);
            }
            else
            {
                wchar_t textureName[64];
                swprintf(textureName, 64, L"%i", packId);
                m_packIcon.setTextureName(textureName, true);
            }
        }
    }
    else
    {
        if (gApp.GetLocalPlayerCount() < 2 && gConsoleUIController.m_screenHeight != 480.0f)
        {
            registerTexture(L"default");
            m_packIcon.setTextureName(L"default", true);
        }

        if (m_packIndex == 0)
        {
            if (m_originalSkinId >= 0 && (m_originalSkinId & 0x7fffffe0) == 0)
                m_skinIndex = static_cast<unsigned int>(m_originalSkinId & 0x1f);
        }
        else if (m_packIndex == 1)
        {
            if (GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) != 0)
            {
                bool skinFound = true;
                if (m_originalSkinId < 0)
                {
                    wchar_t skinPath[256];
                    swprintf(skinPath, 256, L"dlcskin%08d.png",
                             GameSettings::GetPlayerFavoriteSkin(m_playerId,
                                                                 GameSettings::GetPlayerFavoriteSkinsPos(m_playerId))
                                 & s_dlcSkinIdMask);
                    DLCPack* favouritePack = gApp.mDLCManager.getPackContainingSkin(skinPath);
                    bool found = false;
                    if (favouritePack == NULL)
                        skinFound = false;
                    else
                    {
                        favouritePack->getSkinIndexAt(m_currentSkinPath, found);
                        skinFound = found;
                    }
                }
                if (skinFound)
                    m_skinIndex = GameSettings::GetPlayerFavoriteSkinsPos(m_playerId);
            }
            else
            {
                setCharacterBlocked(false);
                setCharacterLocked(false);
            }
        }
    }

    handleSkinIndexChanged();
    SetSkinPackButtonList();
    updatePackDisplay();
}

void UIScene_SkinSelectMenu::tick()
{
    UIScene::tick();
    if (m_packFocusChanged)
    {
        m_packFocusChanged = false;
        handlePackIndexChanged();
    }
    IUIScene_DlcSelectMenu::tick();
}

unsigned int UIScene_SkinSelectMenu::IsFavoriteSkin(int player, int skin)
{
    const unsigned int favouriteCount = GameSettings::GetPlayerFavoriteSkinsCount(player);
    for (unsigned int i = 0; i < favouriteCount; ++i)
    {
        if (static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(i))) == skin)
            return 1;
    }
    return 0;
}

void UIScene_SkinSelectMenu::updateTooltips()
{
    int favouriteTooltip = -1;
    if (m_packIndex == 1)
    {
        if (GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) != 0)
            favouriteTooltip = IDS_TOOLTIPS_FAVORITE_REMOVE;
    }
    else
    {
        favouriteTooltip = static_cast<int>(IDS_TOOLTIPS_FAVORITE_ADD);
        if (m_packIndex == 0)
        {
            if (IsFavoriteSkin(m_playerId, static_cast<int>(m_skinIndex)) != 0)
                favouriteTooltip = IDS_TOOLTIPS_FAVORITE_REMOVE;
        }
        else
        {
            DLCSkinFile* skinFile = NULL;
            if (m_pack != NULL)
                skinFile = static_cast<DLCSkinFile*>(m_pack->getSkinFile(m_skinIndex));
            if (skinFile != NULL)
            {
                std::wstring skinPath = skinFile->getPath();
                if (IsFavoriteSkin(m_playerId, static_cast<int>(CMinecraftApp::getSkinIdFromPath(skinPath))) != 0)
                    favouriteTooltip = IDS_TOOLTIPS_FAVORITE_REMOVE;
                if (!m_pack->hasPurchasedPack() && !skinFile->isFree())
                    favouriteTooltip = -1;
            }
        }
    }

    const int selectTooltip = m_noSkinsToShow ? -1 : IDS_TOOLTIPS_SELECT_SKIN;
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), selectTooltip,
                                     static_cast<int>(IDS_TOOLTIPS_CANCEL), -1, favouriteTooltip, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, IDS_TOOLTIPS_NAVIGATE, false, false);
}

void UIScene_SkinSelectMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_SkinSelectMenu::handleAnimationEnd()
{
    if (!m_skinAnimating)
        return;
    m_skinAnimating = false;
    m_skinUpdatePending = true;
}

std::wstring UIScene_SkinSelectMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"SkinSelectMenuSplit";
    return L"SkinSelectMenu";
}

void UIScene_SkinSelectMenu::InputActionOK(unsigned int player)
{
    const int pad = static_cast<int>(player);
    gConsoleUIController.AnimateKeyPress(pad, 0x14, false, true, false);

    switch (m_packIndex)
    {
    case 0:
        GameSettings::SetPlayerSkin(pad, m_skinIndex, true);
        GameSettings::SetPlayerCape(pad, 0, true);
        m_currentSkinPath = GameSettings::GetPlayerSkinName(pad);
        m_originalSkinId = static_cast<int>(GameSettings::GetPlayerSkinId(pad));
        GameSettings::SetPlayerFavoriteSkinsPos(pad, GameSettings::GetPlayerFavoriteSkinsPos(pad, m_skinIndex));
        setCharacterSelected(true);
        break;

    case 1:
        if (GameSettings::GetPlayerFavoriteSkinsCount(pad) > 0)
        {
            bool isDefaultSkin = false;
            bool licensed = false;
            DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(gApp.mDLCManager.getSkinFile(
                static_cast<int>(GameSettings::GetPlayerFavoriteSkin(pad, static_cast<int>(m_skinIndex))),
                isDefaultSkin, licensed));
            if (isDefaultSkin)
            {
                GameSettings::SetPlayerSkin(
                    pad, GameSettings::GetPlayerFavoriteSkin(pad, static_cast<int>(m_skinIndex)), true);
                GameSettings::SetPlayerCape(pad, 0, true);
                m_currentSkinPath = GameSettings::GetPlayerSkinName(pad);
                m_originalSkinId = static_cast<int>(GameSettings::GetPlayerSkinId(pad));
                GameSettings::SetPlayerFavoriteSkinsPos(pad, static_cast<int>(m_skinIndex));
                setCharacterSelected(true);
            }
            else if (skinFile != NULL)
            {
                if (!Minecraft::GetInstance()->GetMiniGame()->AllowSmallSkins()
                    || (skinFile->getSkinAdjustments().mGameFlags & 1) == 0)
                {
                    GameSettings::SetPlayerSkin(pad, skinFile->getPath());
                    GameSettings::SetPlayerCape(
                        pad, skinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath, L""));
                    m_currentSkinPath = GameSettings::GetPlayerSkinName(pad);
                    m_originalSkinId = static_cast<int>(GameSettings::GetPlayerSkinId(pad));
                    GameSettings::SetPlayerFavoriteSkinsPos(pad, static_cast<int>(m_skinIndex));
                    setCharacterSelected(true);
                }
                else
                {
                    setCharacterSelected(false);
                }
            }
        }
        break;

    default:
        if (m_pack != NULL)
        {
            m_purchasePending = true;
            bool handled = true;
            DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(m_pack->getSkinFile(m_skinIndex));
            if (!skinFile->getParameterAsBool(DLCManager::eDLCParameterType_Free))
            {
                if (!m_pack->hasPurchasedPack())
                {
                    tryPurchaseDlc(m_playerId, handled, eDLCContentType_All);
                }
                else if (!Minecraft::GetInstance()->GetMiniGame()->AllowSmallSkins()
                         || (skinFile->getSkinAdjustments().mGameFlags & 1) == 0)
                {
                    GameSettings::SetPlayerSkin(pad, skinFile->getPath());
                    GameSettings::SetPlayerCape(
                        pad, skinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath, L""));
                    setCharacterSelected(true);
                    m_currentSkinPath = GameSettings::GetPlayerSkinName(pad);
                    m_originalSkinId = static_cast<int>(GameSettings::GetPlayerSkinId(pad));
                    GameSettings::SetPlayerFavoriteSkinsPos(
                        m_playerId, GameSettings::GetPlayerFavoriteSkinsPos(
                                        m_playerId, static_cast<unsigned int>(m_originalSkinId)));
                }
                else
                {
                    setCharacterSelected(false);
                }
            }
            else if (!Minecraft::GetInstance()->GetMiniGame()->AllowSmallSkins()
                     || (skinFile->getSkinAdjustments().mGameFlags & 1) == 0)
            {
                GameSettings::SetPlayerSkin(pad, skinFile->getPath());
                GameSettings::SetPlayerCape(
                    pad, skinFile->getParameterAsString(DLCManager::eDLCParameterType_CapePath, L""));
                setCharacterSelected(true);
                m_currentSkinPath = GameSettings::GetPlayerSkinName(pad);
                m_originalSkinId = static_cast<int>(GameSettings::GetPlayerSkinId(pad));
                GameSettings::SetPlayerFavoriteSkinsPos(
                    m_playerId,
                    GameSettings::GetPlayerFavoriteSkinsPos(m_playerId, static_cast<unsigned int>(m_originalSkinId)));
            }
            else
            {
                setCharacterSelected(false);
            }

            if (handled)
                m_purchasePending = false;
        }
        break;
    }

    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

int UIScene_SkinSelectMenu::ManageFavoritesReturned(void* context, int, C4JStorage::EMessageResult result)
{
    UIScene_SkinSelectMenu* scene = static_cast<UIScene_SkinSelectMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (scene != NULL)
    {
        if (result == C4JStorage::EMessageResult_Accept)
        {
            scene->m_packList.HighlightItem(1, false);
            scene->handlePackIndexChanged();
        }
        else
        {
            scene->m_purchasePending = false;
        }
    }
    return 0;
}

void UIScene_SkinSelectMenu::SetFavoriteSkin(int player, int skin)
{
    const unsigned int favouriteCount = GameSettings::GetPlayerFavoriteSkinsCount(player);
    unsigned int slot = 0;
    if (favouriteCount != 0)
    {
        while (static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(slot))) != skin)
        {
            ++slot;
            if (slot >= favouriteCount)
                break;
        }
    }

    if (favouriteCount != 0 && slot < favouriteCount)
    {
        setCharacterFavourite(false);
        for (unsigned int i = slot; i < favouriteCount - 1; ++i)
            GameSettings::SetPlayerFavoriteSkin(player, static_cast<int>(i),
                                                GameSettings::GetPlayerFavoriteSkin(player, static_cast<int>(i + 1)));
        GameSettings::SetPlayerFavoriteSkin(player, static_cast<int>(favouriteCount - 1), 0xffffffff);
        if (GameSettings::GetPlayerFavoriteSkinsPos(player) == slot)
            GameSettings::SetPlayerFavoriteSkinsPos(
                m_playerId, static_cast<int>(GameSettings::GetPlayerFavoriteSkinsCount(player)));
        return;
    }

    if (favouriteCount == 10)
    {
        int messageParameters[2] = {static_cast<int>(IDS_CONFIRM_OK), IDS_CONFIRM_CANCEL};
        gConsoleUIController.RequestAlertMessage(IDS_MAX_FAVORITES_TITLE, static_cast<int>(IDS_MAX_FAVORITES_TEXT),
                                                 messageParameters, 2, player, ManageFavoritesReturned,
                                                 reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }

    setCharacterFavourite(true);
    GameSettings::SetPlayerFavoriteSkin(player, static_cast<int>(favouriteCount), static_cast<unsigned int>(skin));
    if (m_originalSkinId == skin)
        GameSettings::SetPlayerFavoriteSkinsPos(m_playerId, static_cast<int>(favouriteCount));
}

void UIScene_SkinSelectMenu::InputActionFavorite(unsigned int player)
{
    const int pad = static_cast<int>(player);
    if (m_packIndex == 0)
    {
        SetFavoriteSkin(m_playerId, static_cast<int>(m_skinIndex));
        handleSkinIndexChanged();
    }
    else if (m_packIndex == 1)
    {
        if (GameSettings::GetPlayerFavoriteSkinsCount(pad) == 0)
            return;
        bool isDefaultSkin = false;
        bool licensed = false;
        DLCFile* skinFile = gApp.mDLCManager.getSkinFile(
            static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, static_cast<int>(m_skinIndex))),
            isDefaultSkin, licensed);
        if (skinFile == NULL && !isDefaultSkin)
            return;
        SetFavoriteSkin(m_playerId,
                        static_cast<int>(GameSettings::GetPlayerFavoriteSkin(pad, static_cast<int>(m_skinIndex))));
        const unsigned int favouriteCount = GameSettings::GetPlayerFavoriteSkinsCount(pad);
        if (favouriteCount <= m_skinIndex && favouriteCount != 0)
            m_skinIndex = favouriteCount - 1;
        handleSkinIndexChanged();
    }
    else
    {
        DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(m_pack->getSkinFile(m_skinIndex));
        if (m_pack->hasPurchasedPack() || (skinFile != NULL && skinFile->isFree()))
        {
            SetFavoriteSkin(m_playerId, static_cast<int>(CMinecraftApp::getSkinIdFromPath(skinFile->getPath())));
            updateTooltips();
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        return;
    }

    if (m_originalSkinId >= 0)
        GameSettings::SetPlayerFavoriteSkinsPos(
            m_playerId, GameSettings::GetPlayerFavoriteSkinsPos(pad, static_cast<unsigned int>(m_originalSkinId)));
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_SkinSelectMenu::handleInput(int player, int action, bool repeat, bool pressed, bool released,
                                         bool& handled)
{
    if (m_purchasePending)
        return;

    switch (action)
    {
    case EControllerActions_MenuY:
        if (pressed)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, pressed, released);
            InputActionFavorite(static_cast<unsigned int>(player));
            handled = true;
            return;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        if (pressed)
        {
            sendInputToMovie(action, repeat, pressed, released);
            handled = true;
            return;
        }
        break;
    case EControllerActions_MenuRight:
        if (pressed && !m_skinSliding)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, pressed, released);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            m_skinIndex = getNextSkinIndex(m_skinIndex);
            m_skinSliding = true;
            m_skinAnimating = true;
            m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, true);
            m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
            sendInputToMovie(7, repeat, pressed, released);
            return;
        }
        break;
    case EControllerActions_MenuLeft:
        if (pressed && !m_skinSliding)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, pressed, released);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            m_skinIndex = static_cast<unsigned int>(getPreviousSkinIndex(m_skinIndex));
            m_skinAnimating = true;
            m_skinSliding = true;
            m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, true);
            m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
            sendInputToMovie(6, repeat, pressed, released);
            return;
        }
        break;
    case EControllerActions_MenuOtherStickPress:
        if (pressed)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            m_characters[eCharacter_Current].mRotation = 0;
            m_characters[eCharacter_Current].mPitch = 0;
            return;
        }
        break;
    case EControllerActions_MenuOtherStickUp:
        if (pressed)
        {
            m_characters[eCharacter_Current].CyclePreviousAnimation();
            return;
        }
        break;
    case EControllerActions_MenuOtherStickDown:
        if (pressed)
            m_characters[eCharacter_Current].CycleNextAnimation();
        break;
    case EControllerActions_MenuOtherStickLeft:
        if (pressed)
        {
            m_characters[eCharacter_Current].mRotateForward = true;
            return;
        }
        if (released)
        {
            m_characters[eCharacter_Current].mRotateForward = false;
            return;
        }
        break;
    case EControllerActions_MenuOtherStickRight:
        if (pressed)
        {
            m_characters[eCharacter_Current].mRotateBackward = true;
            return;
        }
        if (released)
        {
            m_characters[eCharacter_Current].mRotateBackward = false;
            return;
        }
        break;
    case EControllerActions_MenuOk:
        if (pressed)
        {
            InputActionOK(static_cast<unsigned int>(player));
            return;
        }
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, pressed, released);
            GameSettings::CheckGameSettingsChanged(true, player);
            navigateBack();
            return;
        }
        break;
    default:
        break;
    }
}

void UIScene_SkinSelectMenu::customDraw(const char* name, fuiRect* rect)
{
    if (m_skinUpdatePending)
    {
        bool frameIsOne;
        {
            std::string charactersName("IggyCharacters");
            frameIsOne = m_movie->getRootNode()->getFuiNode()->getChildByName(charactersName)->currentFrame() == 1;
        }
        if (frameIsOne)
        {
            m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, false);
            m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, false);
            m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, false);
            m_skinSliding = false;
            m_skinUpdatePending = false;
            handleSkinIndexChanged();
        }
    }

    int character = -1;
    std::sscanf(name, "Character%d", &character);
    if (character != -1)
        m_characters[character].render(rect);
}

void UIScene_SkinSelectMenu::HandleDLCMountingComplete()
{
    m_controlTimer.setVisible(false);
    m_controlCharacters.setVisible(true);
    GameSettings::ValidateFavoriteSkins(m_playerId);
    setActivePackIndex();
    m_packListDirty = true;
    handlePackIndexChanged();
    m_purchasePending = false;
    gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
}

void UIScene_SkinSelectMenu::HandleDLCInstalled()
{
    if (gApp.StartInstallDLCProcess(m_playerId) == 0)
    {
        m_purchasePending = false;
        return;
    }
    m_purchasePending = true;
    m_controlTimer.setVisible(true);
    m_controlCharacters.setVisible(false);
}

void UIScene_SkinSelectMenu::HandleDLCLicenseChange()
{
    handleSkinIndexChanged();
}

void UIScene_SkinSelectMenu::handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                              bool repeat, bool released)
{
    if (pressed)
    {
        if (touchBox == 10 && (m_packIndex != 1 || GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) != 0))
        {
            m_touchStartX = touchX;
            m_touchConsumed = false;
        }
        return;
    }

    if (!repeat)
    {
        if (released && touchBox == 10 && !m_touchConsumed)
            InputActionOK(playerId);
        return;
    }

    if (touchBox != 10)
        return;

    if (touchX < m_touchStartX - 50)
    {
        if (!m_skinSliding && !m_touchConsumed)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            m_skinIndex = getNextSkinIndex(m_skinIndex);
            m_skinSliding = true;
            m_skinAnimating = true;
            m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, true);
            m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
            sendInputToMovie(7, false, true, false);
            m_touchConsumed = true;
        }
    }
    else if (touchX > m_touchStartX + 50 && !m_skinSliding && !m_touchConsumed)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
        m_skinIndex = static_cast<unsigned int>(getPreviousSkinIndex(m_skinIndex));
        m_skinSliding = true;
        m_skinAnimating = true;
        m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, true);
        m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
        sendInputToMovie(6, false, true, false);
        m_touchConsumed = true;
    }
}

void UIScene_SkinSelectMenu::handleReload()
{
    m_purchasePending = false;
    m_labelSkinName.init(UIString(L""));
    m_labelSkinOrigin.init(UIString(L""));
    m_skinAnimating = false;
    m_skinUpdatePending = true;
    m_texturePackSkinPackId = 0;
    const unsigned int packIndex = m_packIndex;
    m_packListDirty = true;
    m_hasTexturePackSkinPack = false;

    if (static_cast<unsigned int>(gApp.mDLCManager.getPackCount(DLCManager::eDLCType_Skin)) > 0)
    {
        m_pack = gApp.mDLCManager.getPackContainingSkin(m_currentSkinPath);
        m_selectedPackId = 0;
        if (m_pack != NULL)
            m_selectedPackId = static_cast<int>(m_pack->GetPackId());
    }

    GameSettings::ValidateFavoriteSkins(m_playerId);
    if (static_cast<unsigned int>(GameSettings::GetPlayerFavoriteSkinsCount(m_playerId)) > 0)
        m_packIndex = 1;

    handlePackIndexChanged();
    m_packList.updateChildFocus(static_cast<int>(packIndex));
    handlePackIndexChanged();
    m_packList.HighlightItem(packIndex, false);
}

void UIScene_SkinSelectMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
    {
        m_packList.updateChildFocus(childFocus);
        m_packFocusChanged = true;
    }
}

void UIScene_SkinSelectMenu::handlePress(int, int)
{
}

int UIScene_SkinSelectMenu::getSceneType()
{
    return EUIScene_SkinSelectMenu;
}
