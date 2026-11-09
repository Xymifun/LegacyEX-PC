#include "game/ui/UIComponent_TutorialPopup.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/ui/FJ_TutorialPopup.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIString.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "vendor/_TutorialPopupInfo.hpp"

UIComponent_TutorialPopup::UIComponent_TutorialPopup(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer),
      m_description(),
      m_icon(),
      m_title(),
      m_descriptionLabel(),
      m_titleLabel(),
      m_iconHolderControl(),
      m_exitScreenshotControl()
{
    initialiseMovie();
    m_interactScene = 0;
    m_previousInteractScene = 0;
    m_interactScenePosition = false;
    m_showTitleTimer = false;
    m_showExitScreenshot = false;
    m_icon = ItemInstance::EMPTY;
    m_showIcon = false;
    m_containerMenuDisplayed = false;
    m_splitScreen = false;
    m_iconHolder = EIcons_None;
    m_descriptionLabel.init(UIString(L""));
}

UIComponent_TutorialPopup::~UIComponent_TutorialPopup()
{
}

void UIComponent_TutorialPopup::SetVisible(bool visible)
{
    m_layer->showComponent(0, EUIScene_TutorialPopup, visible);
    if (visible && m_showTitleTimer)
        addTimer(0, gTutorialVisibleTime);
}

void UIComponent_TutorialPopup::setupIconHolder(EIcons icon)
{
    if (m_movie != 0 && m_movie->getRootNode() != 0)
    {
        FJ_TutorialPopup* popup = static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode());
        if (popup != 0)
            popup->SetupIconHolder(static_cast<int>(icon));
    }
    m_iconHolder = icon;
}

std::wstring UIComponent_TutorialPopup::_SetIcon(not_null_ptr<ItemInstance> item, int itemId, int auxValue,
                                                 bool showIcon, const wchar_t* image)
{
    std::wstring text(image);
    bool usesIconHolderPreset = false;
    bool useItemIcon = true;

    if (itemId == -1)
    {
        m_showIcon = showIcon;
        useItemIcon = !item->isEmpty();
    }

    if (useItemIcon)
    {
        m_showIcon = false;
        m_icon = item;
        if (m_icon->isEmpty())
        {
            Item* fallbackItem = Item::byId(itemId);
            if (fallbackItem == 0)
                m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Block::byId(itemId), 1));
            else
                m_icon = not_null_ptr<ItemInstance>(new ItemInstance(fallbackItem, 1));
            m_icon->setAuxValue(auxValue);
        }
    }
    else
    {
        m_icon = ItemInstance::EMPTY;
        std::wstring openTag(L"{*ICON*}");
        std::wstring closeTag(L"{*/ICON*}");
        int iconStart = text.find(openTag);
        int idStart = iconStart + openTag.length();
        if (iconStart > 0 && idStart < static_cast<int>(text.length()))
        {
            int iconEnd = text.find(closeTag, idStart);
            if (idStart < iconEnd && iconEnd < static_cast<int>(text.length()))
            {
                std::wstring idText = text.substr(idStart, iconEnd - idStart);
                std::vector<std::wstring> idParts = stringSplit(idText, L':');
                int iconId = _fromString<int>(idParts[0]);
                int iconAuxValue = 0;
                if (idParts.size() >= 2)
                    iconAuxValue = _fromString<int>(idParts[1]);
                ItemInstance* icon;
                if (Item::byId(iconId) == 0)
                    icon = new ItemInstance(Block::byId(iconId), 1);
                else
                    icon = new ItemInstance(Item::byId(iconId), 1);
                icon->setAuxValue(iconAuxValue);
                m_icon = not_null_ptr<ItemInstance>(icon);
                text.replace(iconStart, (iconEnd - iconStart) + closeTag.length(), L"");
            }
        }
        else if (text.find(L"{*CraftingTableIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::crafting_table, 1, 0));
        }
        else if (text.find(L"{*SticksIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::stick, 1, 0));
        }
        else if (text.find(L"{*PlanksIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::planks, 1, 0));
        }
        else if (text.find(L"{*WoodenShovelIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenShovel, 1, 0));
        }
        else if (text.find(L"{*WoodenHatchetIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenAxe, 1, 0));
        }
        else if (text.find(L"{*WoodenPickaxeIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenPickaxe, 1, 0));
        }
        else if (text.find(L"{*FurnaceIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::furnace, 1, 0));
        }
        else if (text.find(L"{*WoodenDoorIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenDoor, 1, 0));
        }
        else if (text.find(L"{*TorchIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::torch, 1, 0));
        }
        else if (text.find(L"{*BoatIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::boat, 1, 0));
        }
        else if (text.find(L"{*FishingRodIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::fishingRod, 1, 0));
        }
        else if (text.find(L"{*FishIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::fish, 1, 0));
        }
        else if (text.find(L"{*MinecartIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::minecart, 1, 0));
        }
        else if (text.find(L"{*RailIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::rail, 1, 0));
        }
        else if (text.find(L"{*PoweredRailIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::golden_rail, 1, 0));
        }
        else if (text.find(L"{*StructuresIcon*}") != std::wstring::npos)
        {
            usesIconHolderPreset = true;
            setupIconHolder(static_cast<EIcons>(9));
        }
        else if (text.find(L"{*ToolsIcon*}") != std::wstring::npos)
        {
            usesIconHolderPreset = true;
            setupIconHolder(static_cast<EIcons>(10));
        }
        else if (text.find(L"{*BannerIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::banner, 1, 0));
        }
        else if (text.find(L"{*SlimeBlockIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::slime, 1, 0));
        }
        else if (text.find(L"{*PistonIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::piston, 1, 0));
        }
        else if (text.find(L"{*BlazePowderIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::blazePowder, 1, 0));
        }
        else if (text.find(L"{*ElytraIcon*}") != std::wstring::npos)
        {
            m_icon = not_null_ptr<ItemInstance>(new ItemInstance(Items::elytra, 1, 0));
        }
        else
        {
            m_icon = ItemInstance::EMPTY;
        }
    }

    if (!usesIconHolderPreset && !m_icon->isEmpty())
        setupIconHolder(EIcons_None);

    m_iconHolderControl.setVisible(usesIconHolderPreset || !m_icon->isEmpty());
    return text;
}

std::wstring UIComponent_TutorialPopup::_SetImage(std::wstring& image)
{
    return image;
}

void UIComponent_TutorialPopup::ParseDescription(int, std::wstring& description)
{
    description = replaceAll(description, L"{*CraftingTableIcon*}", L"");
    description = replaceAll(description, L"{*SticksIcon*}", L"");
    description = replaceAll(description, L"{*PlanksIcon*}", L"");
    description = replaceAll(description, L"{*WoodenShovelIcon*}", L"");
    description = replaceAll(description, L"{*WoodenHatchetIcon*}", L"");
    description = replaceAll(description, L"{*WoodenPickaxeIcon*}", L"");
    description = replaceAll(description, L"{*FurnaceIcon*}", L"");
    description = replaceAll(description, L"{*WoodenDoorIcon*}", L"");
    description = replaceAll(description, L"{*TorchIcon*}", L"");
    description = replaceAll(description, L"{*MinecartIcon*}", L"");
    description = replaceAll(description, L"{*BoatIcon*}", L"");
    description = replaceAll(description, L"{*FishingRodIcon*}", L"");
    description = replaceAll(description, L"{*FishIcon*}", L"");
    description = replaceAll(description, L"{*RailIcon*}", L"");
    description = replaceAll(description, L"{*PoweredRailIcon*}", L"");
    description = replaceAll(description, L"{*StructuresIcon*}", L"");
    description = replaceAll(description, L"{*ToolsIcon*}", L"");
    description = replaceAll(description, L"{*StoneIcon*}", L"");
    description = replaceAll(description, L"{*BannerIcon*}", L"");
    description = replaceAll(description, L"{*SlimeBlockIcon*}", L"");
    description = replaceAll(description, L"{*PistonIcon*}", L"");
    description = replaceAll(description, L"{*BlazePowderIcon*}", L"");
    description = replaceAll(description, L"{*ElytraIcon*}", L"");

    std::wstring::size_type exitPicture = description.find(L"{*EXIT_PICTURE*}");
    description = replaceAll(description, L"{*EXIT_PICTURE*}", L"");
    m_exitScreenshotControl.setVisible(exitPicture != std::wstring::npos);

    m_title = description;
}

void UIComponent_TutorialPopup::_SetDescription(UIScene* scene, const std::wstring& title,
                                                const std::wstring& description, bool showTitle,
                                                bool showExitScreenshot, int displayTime)
{
    m_interactScene = scene;
    m_description = description;
    if (scene != m_previousInteractScene)
        m_previousInteractScene = 0;

    if (title.empty())
    {
        SetVisible(false);
        addTimer(1, 500);
        killTimer(0);
        return;
    }

    SetVisible(true);
    addTimer(1, 500);
    if (showTitle)
        addTimer(0, displayTime < 0 ? gTutorialVisibleTime : displayTime);
    else
        killTimer(0);
    m_showTitleTimer = showTitle;
    m_showExitScreenshot = showExitScreenshot;

    if (showExitScreenshot)
    {
        std::wstring titleText = L10N::GetString(IDS_TUTORIAL_REMINDER);
        titleText += title;
        stripWhitespaceForHtml(titleText, true);

        wchar_t colorTag[40];
        swprintf(colorTag, 40, L"<font color=\"#%08x\">", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
        titleText = colorTag + titleText;

        m_descriptionLabel.setLabel(UIString(titleText), true, false);
    }
    else
    {
        std::wstring titleText = title;
        stripWhitespaceForHtml(titleText, true);

        wchar_t colorTag[40];
        swprintf(colorTag, 40, L"<font color=\"#%08x\">", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
        titleText = colorTag + titleText;

        m_descriptionLabel.setLabel(UIString(titleText), true, false);
    }

    m_titleLabel.setLabel(UIString(description), true, false);
    m_titleLabel.setVisible(!description.empty());

    m_splitScreen = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitscreenNicknames) != 0;
    bool adjustLayout = m_splitScreen && !m_containerMenuDisplayed;
    static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode())->AdjustLayout(adjustLayout);
}

void UIComponent_TutorialPopup::SetTutorialDescription(_TutorialPopupInfo* info)
{
    m_interactScene = info->scene;
    std::wstring text = _SetIcon(info->icon, info->iconItemId, info->iconAuxValue, info->showIcon, info->title);
    text = _SetImage(text);
    ParseDescription(m_playerId, text);
    std::wstring formattedTitle = gApp.FormatHTMLString(m_playerId, m_title, -1);
    if (formattedTitle.empty())
        _SetDescription(info->scene, L"", L"", info->showTitle, info->showExitScreenshot, info->displayTime);
    else
        _SetDescription(info->scene, formattedTitle, info->description, info->showTitle, info->showExitScreenshot,
                        info->displayTime);
}

void UIComponent_TutorialPopup::RemoveInteractSceneReference(UIScene* scene)
{
    if (m_interactScene == scene)
        m_interactScene = 0;
}

void UIComponent_TutorialPopup::UpdateTutorialPopup()
{
    if (m_splitScreen
        != (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitscreenNicknames) != 0))
    {
        m_splitScreen = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitscreenNicknames)
                        != 0;
        handleReload();
    }
}

bool UIComponent_TutorialPopup::IsVisible()
{
    return m_layer->isComponentVisible(EUIScene_TutorialPopup);
}

unsigned int UIComponent_TutorialPopup::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_titleLabel.setupControl(this, root, std::string("Title"));
        m_controls.push_back(&m_titleLabel);
    }
    {
        m_descriptionLabel.setupControl(this, root, std::string("Description"));
        m_controls.push_back(&m_descriptionLabel);
    }
    {
        m_iconHolderControl.setupControl(this, root, std::string("IconHolder"));
        m_controls.push_back(&m_iconHolderControl);
    }
    {
        m_exitScreenshotControl.setupControl(this, root, std::string("ExitScreenShot"));
        m_controls.push_back(&m_exitScreenshotControl);
    }
    return 1;
}

std::wstring UIComponent_TutorialPopup::getMoviePath()
{
    switch (m_layer->getViewport())
    {
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_TopLeftQuarter:
    case C4JRender::eViewportType_TopRightQuarter:
    case C4JRender::eViewportType_BottomLeftQuarter:
    case C4JRender::eViewportType_BottomRightQuarter:
        return L"TutorialPopupSplit";
    default:
        return L"TutorialPopup";
    }
}

void UIComponent_TutorialPopup::handleReload()
{
    if (IsVisible())
    {
        std::wstring formattedTitle = gApp.FormatHTMLString(m_playerId, m_title, -1);
        if (formattedTitle.empty())
        {
            _SetDescription(m_interactScene, L"", L"", m_showTitleTimer, m_showExitScreenshot, -1);
        }
        else
        {
            _SetDescription(m_interactScene, formattedTitle, m_description, m_showTitleTimer, m_showExitScreenshot, -1);
        }
    }

    bool adjustSplit = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitscreenNicknames) != 0
                       && !m_containerMenuDisplayed;
    FJ_TutorialPopup* popup = static_cast<FJ_TutorialPopup*>(m_movie->getRootNode()->getFuiNode());
    popup->AdjustLayout(adjustSplit);
    setupIconHolder(m_iconHolder);
}

void UIComponent_TutorialPopup::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (viewport != C4JRender::eViewportType_FullScreen)
    {
        int x = 0;
        int y = 0;
        switch (viewport)
        {
        case C4JRender::eViewportType_BottomLeftQuarter:
            y = static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f);
            break;
        case C4JRender::eViewportType_TopHalf:
        case C4JRender::eViewportType_RightHalf:
        case C4JRender::eViewportType_TopRightQuarter:
            x = static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f);
            break;
        case C4JRender::eViewportType_BottomHalf:
        case C4JRender::eViewportType_BottomRightQuarter:
            x = static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f);
            y = static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f);
            break;
        default:
            break;
        }

        switch (viewport)
        {
        case C4JRender::eViewportType_TopHalf:
        case C4JRender::eViewportType_LeftHalf:
        case C4JRender::eViewportType_RightHalf:
        case C4JRender::eViewportType_TopLeftQuarter:
        case C4JRender::eViewportType_TopRightQuarter:
            y += static_cast<int>(getSafeZoneHalfHeight());
            break;
        default:
            break;
        }

        switch (viewport)
        {
        case C4JRender::eViewportType_TopHalf:
        case C4JRender::eViewportType_BottomHalf:
        case C4JRender::eViewportType_RightHalf:
        case C4JRender::eViewportType_TopRightQuarter:
        case C4JRender::eViewportType_BottomRightQuarter:
            x -= static_cast<int>(getSafeZoneHalfWidth());
            break;
        default:
            break;
        }

        gConsoleUIController.setupRenderPosition(x, y);

        gFui->render(m_movie, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    }
    else
    {
        UIScene::render(width, height, viewport);
    }
}

void UIComponent_TutorialPopup::customDraw(const char*, fuiRect* rect)
{
    if (m_icon->getItem() != 0)
    {
        if (m_icon->getCount() == 0)
            m_icon->setCount(1);
        customDrawSlotControlFui(rect, m_playerId, m_icon, 1.0f, m_icon->getItem()->isFoil(m_icon) || m_showIcon, false,
                                 false);
    }
}

void UIComponent_TutorialPopup::UpdateInteractScenePosition(bool visible)
{
    if (m_interactScene == 0)
        return;

    bool craftingMenu = m_interactScene->getSceneType() == EUIScene_CraftingMenu2x2
                        || m_interactScene->getSceneType() == EUIScene_CraftingMenu;
    bool creativeMenu = m_interactScene->getSceneType() == EUIScene_CreativeMenu;
    bool tradingMenu = m_interactScene->getSceneType() == EUIScene_TradingMenu;
    if (Minecraft::GetInstance()->mLocalPlayers[m_playerId]->mViewport > C4JRender::eViewportType_BottomHalf
        && (craftingMenu || creativeMenu || tradingMenu))
        return;

    if (m_previousInteractScene != m_interactScene)
    {
        if (visible)
        {
            m_interactScene->slideLeft();
            m_interactScenePosition = visible;
            m_previousInteractScene = m_interactScene;
        }
    }
    else if (m_interactScenePosition != visible)
    {
        if (visible)
        {
            m_interactScene->slideLeft();
            m_interactScenePosition = visible;
            m_previousInteractScene = m_interactScene;
        }
        else
        {
            m_interactScene->slideRight();
            m_interactScenePosition = visible;
            m_previousInteractScene = m_interactScene;
        }
    }
}

void UIComponent_TutorialPopup::handleTimerComplete(int timerId)
{
    switch (timerId)
    {
    case 0:
        SetVisible(false);
        killTimer(0);
        addTimer(1, 500);
        break;
    case 1:
        UpdateInteractScenePosition(IsVisible());
        killTimer(1);
        break;
    }
}

int UIComponent_TutorialPopup::getSceneType()
{
    return EUIScene_TutorialPopup;
}
bool UIComponent_TutorialPopup::stealsFocus()
{
    return false;
}
bool UIComponent_TutorialPopup::hasFocus(int)
{
    return false;
}
bool UIComponent_TutorialPopup::hidesLowerScenes()
{
    return false;
}
