#include "game/ui/UIControl_EnchantmentButton.hpp"

#include <algorithm>
#include <iterator>
#include <sstream>

#include "game/client/render/Font.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/item/Enchantment.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/ui/FJ_EnchantingButton.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIScene_EnchantingMenu.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"


extern CConsoleMinecraftApp gApp;
static UIControl_EnchantmentButton::EnchantmentNames g_enchantmentNames;

UIControl_EnchantmentButton::EnchantmentNames::EnchantmentNames()
{
    std::wstring source(
        L"the elder scrolls klaatu berata niktu xyzzy bless curse light darkness fire air earth water hot dry cold wet ignite snuff embiggen twist shorten stretch fiddle destroy imbue galvanize enchant free limited range of towards inside sphere cube self other ball mental physical grow shrink demon elemental spirit animal creature beast humanoid undead fresh stale phnglui mglwnafh cthulhu rlyeh wgahnagl fhtagn baguette");
    std::wistringstream stream(source);
    std::copy(std::istream_iterator<std::wstring, wchar_t>(stream), std::istream_iterator<std::wstring, wchar_t>(),
              std::back_inserter(m_names));
}

std::wstring UIControl_EnchantmentButton::EnchantmentNames::getRandomName()
{
    const int count = nextInt(2) + 3;
    std::wstring result(L"");
    for (int i = 0; i < count; ++i)
    {
        if (i > 0)
        {
            result.append(L" ");
        }
        result.append(m_names[nextInt((int)m_names.size())]);
    }
    return result;
}

UIControl_EnchantmentButton::UIControl_EnchantmentButton() : UIControl_Button()
{
    m_state = 0;
    m_level = 0;
    m_index = 0;
    m_name = L"";
    m_focus = false;
    m_colour = (int)(gApp.GetHTMLColour(eMinecraftColour_Color_EnchantText) | 0xff000000);
    m_focusColour = (int)(gApp.GetHTMLColour(eMinecraftColour_Color_EnchantTextFocus) | 0xff000000);
    m_inactiveColour = (int)(gApp.GetHTMLColour(eMinecraftColour_Color_EnchantTextDisabled) | 0xff000000);
}

bool UIControl_EnchantmentButton::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_EnchantmentButton;
    return UIControl_Button::setupControl(scene, node, name);
}

void UIControl_EnchantmentButton::updateState()
{
    UIScene_EnchantingMenu* scene = static_cast<UIScene_EnchantingMenu*>(m_scene);
    EnchantmentMenu* menu = scene->getMenu();
    const int level = menu->mCosts[m_index];
    const int goldCount = menu->getGoldCount();
    Enchantment* enchantment = Enchantment::byId(menu->mEnchantmentIds[m_index]);
    Minecraft* minecraft = Minecraft::GetInstance();
    int state;
    if ((enchantment == NULL || goldCount < m_index + 1
         || minecraft->mLocalPlayers[scene->getPad()]->mExperienceLevel < level)
        && !minecraft->mLocalPlayers[scene->getPad()]->mAbilities.mInstabuild)
    {
        state = 0;
    }
    else
    {
        state = 1;
        if (m_focus)
        {
            state = 2;
        }
    }
    if (level != m_level)
    {
        setLabel(UIString(_toString(level)), false, false);
        m_level = level;
        m_name = g_enchantmentNames.getRandomName();
    }
    if (enchantment == NULL || level == 0)
    {
        state = 0;
        setLabel(UIString(L""), false, false);
    }
    if (state != m_state)
    {
        FJ_EnchantingButton* button = static_cast<FJ_EnchantingButton*>(m_fuiRenderNode->getFuiNode());
        button->ChangeState(state);
        m_state = state;
    }
}

void UIControl_EnchantmentButton::tick()
{
    updateState();
    UIControl_Button::tick();
}

void UIControl_EnchantmentButton::init(int index)
{
    m_index = index;
}

void UIControl_EnchantmentButton::ReInit()
{
    UIControl_Button::ReInit();
    m_state = 0;
    m_level = 0;
    m_focus = false;
    updateState();
}

void UIControl_EnchantmentButton::render(fuiRect* rect)
{
    UIScene_EnchantingMenu* scene = static_cast<UIScene_EnchantingMenu*>(m_scene);
    EnchantmentMenu* menu = scene->getMenu();
    GlStateManager::scalef((rect->xMax - rect->xMin) / m_width, (rect->yMax - rect->yMin) / m_height, 1.0F);
    const bool splitScreen = gApp.GetLocalPlayerCount() > 1;
    float scale;
    if (scene->m_movieResolution == 0)
    {
        scale = splitScreen ? 3.0F : 3.5F;
    }
    else
    {
        scale = splitScreen ? 2.0F : 2.5F;
    }
    GlStateManager::scalef(scale, scale, scale);
    const int level = menu->mCosts[m_index];
    const int goldCount = menu->getGoldCount();
    Enchantment* enchantment = Enchantment::byId(menu->mEnchantmentIds[m_index]);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    if (enchantment != NULL && level != 0)
    {
        GlStateManager::enableAlphaTest();
        GlStateManager::alphaFunc(4, 0.1F);
        Minecraft* minecraft = Minecraft::GetInstance();
        std::wstring levelText = _toString(level);
        Font* font = minecraft->mAlternateFont;
        int colour = m_colour;
        if ((goldCount < m_index + 1 || minecraft->mLocalPlayers[scene->getPad()]->mExperienceLevel < level)
            && !minecraft->mLocalPlayers[scene->getPad()]->mAbilities.mInstabuild)
        {
            colour = m_inactiveColour;
        }
        else if (m_focus)
        {
            colour = m_focusColour;
        }
        font->drawWordWrap(m_name, 0, 0, (int)(m_width / scale), colour, (int)(m_height / scale));
        GlStateManager::disableAlphaTest();
    }
    GlStateManager::disableRescaleNormal();
}

void UIControl_EnchantmentButton::SetEnchantmentLevel(int level)
{
    FJ_EnchantingButton* button = static_cast<FJ_EnchantingButton*>(m_fuiRenderNode->getFuiNode());
    button->SetEnchantmentLevel(level);
}

bool UIControl_EnchantmentButton::IsInactive()
{
    return m_state == 0;
}

void UIControl_EnchantmentButton::setFocus(bool focus)
{
    m_focus = focus;
    updateState();
}
