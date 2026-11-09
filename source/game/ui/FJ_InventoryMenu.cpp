#include "game/ui/FJ_InventoryMenu.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_MobEffect.hpp"
#include "game/util/Math.hpp"

static const int s_effectSlots[28] = {-1, 12, 9,  2,  14, 11, 5,  3,  6,  13, 16, 7,  8,  1,
                                      0,  4,  10, 15, 17, 18, 19, 20, 21, 22, 23, -1, 25, 24};

FJ_InventoryMenu::FJ_InventoryMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
    m_inventoryHidden = false;
}

FJ_AbstractContainerDocument* FJ_InventoryMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_InventoryMenu(renderNode);
}

unsigned int FJ_InventoryMenu::getType()
{
    return 12;
}

void FJ_InventoryMenu::InitEffectsGroup()
{
    char name[64];
    for (int index = 0; index < 27; ++index)
    {
        std::sprintf(name, "EffectsGroup.Effect%d", index + 1);
        m_effectNodes[index] = getChildByName(std::string(name));
        m_effectNodes[index]->asFJ_MobEffect()->SetupEffectType(index + 2);
        m_effectNodes[index]->setVisible(false);
        m_effectActive[index] = 0;
        m_effectRefreshed[index] = 0;
    }
    m_effectTopY = static_cast<int>(m_effectNodes[0]->getY());
    m_effectNextY = static_cast<int>(m_effectNodes[1]->getY());
    m_effectBottomY = static_cast<int>(m_effectNodes[6]->getY());
}

void FJ_InventoryMenu::init(FJ_Event* event)
{
    FJ_AbstractContainerDocument::init(event);
    InitEffectsGroup();
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"))->asMovieClip();
    FJ_FuiNode* equipment = mainPanel->getChildByName(std::string("EquipmentHolder"))->asMovieClip();
    FJ_FuiNode* headSlot = equipment->getChildByName(std::string("Slot_Head"))->asMovieClip();
    FJ_FuiNode* bodySlot = equipment->getChildByName(std::string("Slot_Body"))->asMovieClip();
    FJ_FuiNode* legsSlot = equipment->getChildByName(std::string("Slot_Legs"))->asMovieClip();
    FJ_FuiNode* feetSlot = equipment->getChildByName(std::string("Slot_Feet"))->asMovieClip();
    m_armorSlots[0] = headSlot->getChildByName(std::string("Icon_Head"))->asMovieClip();
    m_armorSlots[1] = bodySlot->getChildByName(std::string("Icon_Body"))->asMovieClip();
    m_armorSlots[2] = legsSlot->getChildByName(std::string("Icon_Legs"))->asMovieClip();
    m_armorSlots[3] = feetSlot->getChildByName(std::string("Icon_Feet"))->asMovieClip();
}

void FJ_InventoryMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
    {
        return;
    }
    setX(static_cast<float>(-right));
    setY(static_cast<float>(top));
}

void FJ_InventoryMenu::RepositionEffects()
{
    int y = m_effectTopY;
    int spacing = m_effectTopY - m_effectNextY;
    int active = 0;
    for (int index = 0; index < 27; ++index)
    {
        if (m_effectActive[index] != 0)
        {
            ++active;
        }
    }
    const int visibleLimit = m_inventoryHidden ? 4 : 7;
    if (active > visibleLimit)
    {
        spacing = Math::round(static_cast<float>(m_effectTopY - m_effectBottomY) / static_cast<float>(active - 1));
    }
    for (int index = 0; index < 27; ++index)
    {
        if (m_effectActive[index] != 0)
        {
            m_effectNodes[index]->setY(static_cast<float>(y));
            y -= spacing;
        }
    }
}

void FJ_InventoryMenu::HideInventory()
{
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"))->asMovieClip();
    FJ_FuiNode* backgroundPanel = mainPanel->getChildByName(std::string("BackgroundPanel"))->asMovieClip();
    int oldHeight = static_cast<int>(backgroundPanel->getChildAt(0)->getHeight());
    FJ_AbstractContainerDocument::HideInventory();
    int newHeight = static_cast<int>(backgroundPanel->getChildAt(0)->getHeight());
    m_effectTopY += newHeight - oldHeight;
    m_effectNextY += newHeight - oldHeight;
    m_inventoryHidden = true;
    RepositionEffects();
}

void FJ_InventoryMenu::SetClassicCrafting(bool classicCrafting)
{
    if (!classicCrafting)
    {
        FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"))->asMovieClip();
        FJ_FuiNode* armorList = getChildByName(std::string("MainPanel.armorList"))->asMovieClip();
        FJ_FuiNode* iggyPlayer = getChildByName(std::string("MainPanel.iggy_player"))->asMovieClip();
        mainPanel->getChildByName(std::string("craftingLabel"))->setVisible(false);
        mainPanel->removeChild(mainPanel->getChildByName(std::string("craftingList")), true);
        mainPanel->removeChild(mainPanel->getChildByName(std::string("Result")), true);
        mainPanel->removeChild(mainPanel->getChildByName(std::string("CraftingArrow")), true);
        const int armorListX = static_cast<int>(armorList->getX());
        const int halfSpan = Math::round((armorList->getWidth()
                                          + (iggyPlayer->getX() - (armorList->getX() + armorList->getWidth()))
                                          + iggyPlayer->getWidth())
                                         * 0.5f);
        const int halfBackground = Math::round(mainPanel->getChildByName(std::string("BackgroundPanel"))->getWidth()
                                               * 0.5f);
        const float offset = static_cast<float>(halfBackground - armorListX - halfSpan);
        armorList->incX(offset);
        mainPanel->getChildByName(std::string("EquipmentHolder"))->incX(offset);
        iggyPlayer->incX(offset);
        mainPanel->getChildByName(std::string("PlayerBackground"))->incX(offset);
        mainPanel->getChildByName(std::string("ShieldSlot"))->incX(offset);
        mainPanel->getChildByName(std::string("shieldList"))->incX(offset);
    }
}

void FJ_InventoryMenu::AddEffect(int effect, const std::wstring& name, int duration)
{
    if (effect != 0)
    {
        const int slot = s_effectSlots[effect];
        m_effectNodes[slot]->asFJ_MobEffect()->Init(name, duration);
        m_effectNodes[slot]->setVisible(true);
        m_effectActive[slot] = 1;
        m_effectRefreshed[slot] = 0;
        RepositionEffects();
    }
}

unsigned int FJ_InventoryMenu::UpdateEffects(int count, int* effects)
{
    unsigned int changed = 0;
    if (count > 0)
    {
        for (int index = 0; index + 1 < count; index += 2)
        {
            const int slot = s_effectSlots[effects[index]];
            const int duration = effects[index + 1];
            m_effectNodes[slot]->asFJ_MobEffect()->SetDuration(duration);
            changed = 1;
            m_effectRefreshed[slot] = 1;
        }
    }
    for (unsigned int index = 0; index < 27; ++index)
    {
        if (m_effectActive[index] != 0 && m_effectRefreshed[index] == 0)
        {
            if (static_cast<int>(m_effectNodes[index]->asFJ_MobEffect()->getTimeLeft()) <= 0)
            {
                m_effectNodes[index]->setVisible(false);
                m_effectActive[index] = 0;
                RepositionEffects();
            }
            else
            {
                m_effectNodes[index]->asFJ_MobEffect()->SetDuration(
                    static_cast<int>(m_effectNodes[index]->asFJ_MobEffect()->getTimeLeft()) - 1);
                changed = 1;
            }
        }
        m_effectRefreshed[index] = 0;
    }
    return changed;
}

void FJ_InventoryMenu::ShowArmorSlot(int slot, bool visible)
{
    m_armorSlots[slot]->setVisible(visible);
}
