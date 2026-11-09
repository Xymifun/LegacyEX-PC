#include "game/ui/UIScene_TradingMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/Merchant.hpp"
#include "game/core/Rarity.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/core/Slot.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/network/TradeItemPacket.hpp"
#include "game/ui/FJ_TradingMenu.hpp"
#include "game/ui/MerchantMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/GameSettings.hpp"

#include <cstdio>

extern ConsoleUIController gConsoleUIController;

struct TradingMenuData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Merchant> merchant;
    Level* level;
};

UIScene_TradingMenu::UIScene_TradingMenu(int iPad, void* initData, UILayer* layer)
    : UIScene(iPad, layer),
      m_merchant(),
      m_trades(),
      m_mainPanel(),
      m_tradingBar(),
      m_request1(),
      m_request2(),
      m_hotBar(),
      m_inventory(),
      m_inventoryLabel(),
      m_villagerText(),
      m_requiredLabel(),
      m_request1Label(),
      m_request2Label()
{
    initialiseMovie();
    m_tradeCount = 0;
    m_tradeFocusSet = false;
    m_scrollRightShown = true;
    m_merchantMenu = NULL;
    m_tradeRow = 0;
    m_tradeOffset = 0;
    m_scrollLeftShown = true;
    m_villagerText.init(UIString(L""));
    m_inventoryLabel.init(UIString(L10N::GetString(IDS_INVENTORY)));
    m_requiredLabel.init(UIString(L10N::GetString(IDS_REQUIRED_ITEMS_FOR_TRADE)));
    m_request1Label.init(UIString(L""));
    m_request2Label.init(UIString(L""));

    TradingMenuData* sceneData = static_cast<TradingMenuData*>(initData);
    m_merchant = sceneData->merchant;

    if (Minecraft::GetInstance()->mGameModes[iPad] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[iPad]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_TradingMenu, this);
        }
    }

    m_merchantMenu = new MerchantMenu(sceneData->inventory, sceneData->merchant, sceneData->level);
    Minecraft::GetInstance()->mLocalPlayers[m_playerId].get()->mContainerMenu = m_merchantMenu;

    m_request1.addSlots(0x27, 1);
    m_request2.addSlots(0x28, 1);
    m_tradingBar.addSlots(0x29, 7);
    m_inventory.addSlots(3, 0x1b);
    m_hotBar.addSlots(0x1e, 9);

    delete sceneData;

    gConsoleUIController.OverrideSFX(m_playerId, 0, true);
    gConsoleUIController.OverrideSFX(m_playerId, 0x14, true);
    gConsoleUIController.OverrideSFX(m_playerId, 0xb, true);
    gConsoleUIController.OverrideSFX(m_playerId, 10, true);
    gConsoleUIController.OverrideSFX(m_playerId, 7, true);
    gConsoleUIController.OverrideSFX(m_playerId, 6, true);
    gConsoleUIController.OverrideSFX(m_playerId, 4, true);
    gConsoleUIController.OverrideSFX(m_playerId, 5, true);
    gApp.SetRichPresenceContext(iPad, 0xd);
}

UIScene_TradingMenu::~UIScene_TradingMenu()
{
    delete m_merchantMenu;
    if (Minecraft::GetInstance()->mLocalPlayers[m_playerId].get() != 0)
    {
        if (Minecraft::GetInstance()->mLocalPlayers[m_playerId].get()->mContainerMenu->m_containerId
            == m_merchantMenu->m_containerId)
            Minecraft::GetInstance()->mLocalPlayers[m_playerId].get()->closeContainer();
    }
}

unsigned int UIScene_TradingMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_mainPanel.setupControl(this, currentNode, std::string("MainPanel"));
        m_controls.push_back(&m_mainPanel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_tradingBar.setupControl(this, currentNode, std::string("TradingBar"));
            m_controls.push_back(&m_tradingBar);
        }
        {
            m_request1.setupControl(this, currentNode, std::string("Request1"));
            m_controls.push_back(&m_request1);
        }
        {
            m_request2.setupControl(this, currentNode, std::string("Request2"));
            m_controls.push_back(&m_request2);
        }
        {
            m_villagerText.setupControl(this, currentNode, std::string("VillagerText"));
            m_controls.push_back(&m_villagerText);
        }
        {
            m_requiredLabel.setupControl(this, currentNode, std::string("RequiredLabel"));
            m_controls.push_back(&m_requiredLabel);
        }
        {
            m_request1Label.setupControl(this, currentNode, std::string("Request1Label"));
            m_controls.push_back(&m_request1Label);
        }
        {
            m_request2Label.setupControl(this, currentNode, std::string("Request2Label"));
            m_controls.push_back(&m_request2Label);
        }
        {
            m_hotBar.setupControl(this, currentNode, std::string("HotBar"));
            m_controls.push_back(&m_hotBar);
        }
        {
            m_inventory.setupControl(this, currentNode, std::string("Inventory"));
            m_controls.push_back(&m_inventory);
        }
        {
            m_inventoryLabel.setupControl(this, currentNode, std::string("InventoryLabel"));
            m_controls.push_back(&m_inventoryLabel);
        }
        currentNode = parentNode;
    }
    return 1;
}

std::wstring UIScene_TradingMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"TradingMenuSplit") : std::wstring(L"TradingMenu");
}

void UIScene_TradingMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_TRADE, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_TradingMenu::handleDestroy()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[m_playerId] != 0)
    {
        Minecraft* current = Minecraft::GetInstance();
        if (current->mGameModes[m_playerId] != 0)
        {
            Tutorial* tutorial = current->mGameModes[m_playerId]->getTutorial();
            if (tutorial != 0)
                tutorial->changeTutorialState(m_previousTutorialState, 0);
        }
    }

    if (minecraft->mLocalPlayers[m_playerId])
        minecraft->mLocalPlayers[m_playerId].get()->closeContainer();

    gConsoleUIController.OverrideSFX(m_playerId, 0, false);
    gConsoleUIController.OverrideSFX(m_playerId, 0x14, false);
    gConsoleUIController.OverrideSFX(m_playerId, 0xb, false);
    gConsoleUIController.OverrideSFX(m_playerId, 10, false);
    gConsoleUIController.OverrideSFX(m_playerId, 7, false);
    gConsoleUIController.OverrideSFX(m_playerId, 6, false);
    gConsoleUIController.OverrideSFX(m_playerId, 4, false);
    gConsoleUIController.OverrideSFX(m_playerId, 5, false);
}

std::vector<HtmlString>* UIScene_TradingMenu::GetItemDescription(not_null_ptr<ItemInstance> item)
{
    std::vector<HtmlString>* lines = item.get()->getTooltipLines(boost::shared_ptr<Player>(), false);
    if (!lines->empty())
    {
        ItemInstance* instance = item.get();
        lines->at(0).mColour = instance->getRarity()->colour;
    }
    return lines;
}

bool UIScene_TradingMenu::canMake(MerchantRecipe* recipe)
{
    bool canMake = false;
    if (recipe != 0)
    {
        if (recipe->isDeprecated())
            return false;

        not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
        not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();

        boost::shared_ptr<Player> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];

        const int matchesA = player.get()->mInventory.get()->countMatches(buyA);
        const bool haveEnoughA = (matchesA > 0) ? !(matchesA < buyA.get()->getCount()) : buyA.get()->isEmpty();

        const int matchesB = player.get()->mInventory.get()->countMatches(buyB);
        if (matchesB > 0)
            canMake = haveEnoughA && matchesB >= buyB.get()->getCount();
        else
            canMake = haveEnoughA && buyB.get()->isEmpty();
    }
    return canMake;
}

void UIScene_TradingMenu::setTradeRedBox(int trade, bool show)
{
    m_tradingBar.showSlotRedBox(trade, show);
}

void UIScene_TradingMenu::setTitle(const std::wstring& title)
{
    m_villagerText.setLabel(UIString(title), false, false);
}

void UIScene_TradingMenu::setOfferDescription(std::vector<HtmlString>* description)
{
    std::wstring composed = HtmlString::Compose(description);
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_TradingMenu*>(root->getFuiNode())->SetOfferDescription(composed);
}

void UIScene_TradingMenu::setRequest1Name(const std::wstring& name)
{
    m_request1Label.setLabel(UIString(name), false, false);
}

void UIScene_TradingMenu::setRequest2Name(const std::wstring& name)
{
    m_request2Label.setLabel(UIString(name), false, false);
}

void UIScene_TradingMenu::setRequest1RedBox(bool show)
{
    m_request1.showSlotRedBox(0, show);
}

void UIScene_TradingMenu::setRequest2RedBox(bool show)
{
    m_request2.showSlotRedBox(0, show);
}

void UIScene_TradingMenu::updateDisplay()
{
    int tooltipHint = -1;

    MerchantRecipeList* offers = m_merchant.get()->getOffers(Minecraft::GetInstance()->mLocalPlayers[m_playerId]);

    if (offers != 0)
    {
        m_trades.clear();
        int bestIndex = 0x7fffffff;
        int index = 0;
        for (MerchantRecipeList::iterator it = offers->begin(); it != offers->end(); ++it)
        {
            MerchantRecipe* recipe = *it;
            if (!recipe->isDeprecated())
            {
                m_trades.push_back(std::make_pair(recipe, index));
                if (index < bestIndex)
                    bestIndex = index;
            }
            ++index;
        }

        if (!m_tradeFocusSet && bestIndex != 0 && static_cast<unsigned int>(bestIndex) < offers->size())
        {
            m_merchantMenu->setSelectionHint(bestIndex);

            ByteArrayOutputStream byteStream;
            DataOutputStream dataStream(&byteStream);
            dataStream.writeInt(bestIndex);

            ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
            connection->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
                ServerboundCustomPayloadPacket::TRADER_SELECTION_PACKET, byteStream.toByteArray())));
        }

        if (m_trades.size() < static_cast<unsigned int>(m_tradeOffset + 7))
        {
            const int newOffset = static_cast<int>(m_trades.size()) - 7;
            m_tradeOffset = newOffset < 0 ? 0 : newOffset;
        }

        for (int slot = 0; slot < 7; ++slot)
        {
            bool redBox = false;
            const int tradeIndex = m_tradeOffset + slot;
            if (!m_trades.empty() && static_cast<unsigned int>(tradeIndex) < m_trades.size())
                redBox = !canMake(m_trades.at(tradeIndex).first);
            setTradeRedBox(slot, redBox);
        }

        if (m_trades.empty() || static_cast<unsigned int>(m_tradeRow + m_tradeOffset) >= m_trades.size())
        {
            setTitle(m_merchant.get()->getDisplayName());
            setRequest1Name(std::wstring(L""));
            setRequest2Name(std::wstring(L""));
            setRequest1RedBox(false);
            setRequest2RedBox(false);
            std::vector<HtmlString> emptyDescription;
            setOfferDescription(&emptyDescription);
            m_tradeFocusSet = true;
        }
        else
        {
            MerchantRecipe* recipe = m_trades.at(m_tradeRow + m_tradeOffset).first;

            std::wstring villagerText = L10N::GetString(IDS_VILLAGER_OFFERS_ITEM);
            villagerText = replaceAll(villagerText, std::wstring(L"{*VILLAGER_TYPE*}"),
                                      m_merchant.get()->getDisplayName());
            villagerText.replace(villagerText.find(L"%s"), 2, recipe->getSellItem().get()->getHoverName());
            setTitle(villagerText);
            setOfferDescription(GetItemDescription(recipe->getSellItem()));

            not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
            not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
            if (!buyA.get()->isEmpty())
            {
                setRequest1Name(buyA.get()->getHoverName());
                setRequest2Name(buyB.get()->isEmpty() ? std::wstring(L"") : buyB.get()->getHoverName());
            }
            else
            {
                setRequest1Name(std::wstring(L""));
                setRequest2Name(buyB.get()->isEmpty() ? std::wstring(L"") : buyB.get()->getHoverName());
            }

            boost::shared_ptr<Player> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
            const int matchesA = player.get()->mInventory.get()->countMatches(buyA);
            bool haveEnoughA;
            if (matchesA < 1)
            {
                setRequest1RedBox(true);
                haveEnoughA = false;
            }
            else
            {
                setRequest1RedBox(matchesA < buyA.get()->getCount());
                haveEnoughA = buyA.get()->getCount() <= matchesA;
            }

            const int matchesB = player.get()->mInventory.get()->countMatches(buyB);
            if (matchesB < 1)
            {
                setRequest2RedBox(!buyB.get()->isEmpty());
                if (haveEnoughA && buyB.get()->isEmpty())
                    tooltipHint = IDS_TOOLTIPS_TRADE;
            }
            else
            {
                setRequest2RedBox(matchesB < buyB.get()->getCount());
                if (haveEnoughA && buyB.get()->getCount() <= matchesB)
                    tooltipHint = IDS_TOOLTIPS_TRADE;
            }
            m_tradeFocusSet = true;
        }
    }

    gConsoleUIController.SetTooltips(m_playerId, tooltipHint, IDS_TOOLTIPS_EXIT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, false, false);
}

void UIScene_TradingMenu::handleReload()
{
    m_request1.addSlots(0x27, 1);
    m_request2.addSlots(0x28, 1);
    m_tradingBar.addSlots(0x29, 7);
    m_inventory.addSlots(3, 0x1b);
    m_hotBar.addSlots(0x1e, 9);
    updateDisplay();
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_TradingMenu*>(root->getFuiNode())->SetSelectorSlot(m_tradeRow);
}

void UIScene_TradingMenu::showScrollRightArrow(bool show)
{
    if (m_scrollRightShown != show)
    {
        fuiRenderNode* root = m_movie->getRootNode();
        static_cast<FJ_TradingMenu*>(root->getFuiNode())->ShowScrollRightArrow(show);
        m_scrollRightShown = show;
    }
}

void UIScene_TradingMenu::showScrollLeftArrow(bool show)
{
    if (m_scrollLeftShown != show)
    {
        fuiRenderNode* root = m_movie->getRootNode();
        static_cast<FJ_TradingMenu*>(root->getFuiNode())->ShowScrollLeftArrow(show);
        m_scrollLeftShown = show;
    }
}

void UIScene_TradingMenu::handleTick()
{
    Merchant* merchant = m_merchant.get();
    MerchantRecipeList* offers = merchant->getOffers(Minecraft::GetInstance()->mLocalPlayers[m_playerId]);

    if (offers != 0 && !m_tradeFocusSet)
        updateDisplay();

    const unsigned int tradeCount = static_cast<unsigned int>(m_trades.size());
    showScrollRightArrow(static_cast<unsigned int>(m_tradeOffset + 7) < tradeCount);
    showScrollLeftArrow(m_tradeOffset > 0);

    if (!m_merchant.get()->isAvailableToTrade())
    {
        gConsoleUIController.ShowTooltip(m_playerId, 2, false);
        gConsoleUIController.ShowTooltip(m_playerId, 1, false);
        gConsoleUIController.ShowTooltip(m_playerId, 0, false);
        gConsoleUIController.ShowTooltip(m_playerId, 7, false);
        gConsoleUIController.CloseUIScenes(m_playerId, false);
    }
}

void UIScene_TradingMenu::tick()
{
    UIScene::tick();
    handleTick();
}

void UIScene_TradingMenu::moveSelector(bool right)
{
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_TradingMenu*>(root->getFuiNode())->MoveSelector(right);
}

bool UIScene_TradingMenu::handleKeyDown(int iPad, int action, bool)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[m_playerId] != 0)
    {
        Tutorial* tutorial = minecraft->mGameModes[m_playerId]->getTutorial();
        if (tutorial != 0)
        {
            tutorial->handleUIInput(action);
            if (gConsoleUIController.IsTutorialVisible(m_playerId) && !tutorial->popupPending)
            {
                GameModeConstraints* constraints = minecraft->mGameModes[m_playerId]->getGameModeConstraints();
                if (!constraints->isInputAllowed(action))
                    return false;
            }
        }
    }

    int selectedIndex;
    switch (action)
    {
    case EControllerActions_MenuB:
        gConsoleUIController.ShowTooltip(iPad, 2, false);
        gConsoleUIController.ShowTooltip(iPad, 1, false);
        gConsoleUIController.ShowTooltip(iPad, 0, false);
        gConsoleUIController.ShowTooltip(iPad, 7, false);
        gConsoleUIController.CloseUIScenes(iPad, false);
        return true;

    case EControllerActions_MenuA:
    {
        if (m_trades.empty())
            return true;

        selectedIndex = m_tradeRow + m_tradeOffset;
        if (static_cast<unsigned int>(selectedIndex) >= m_trades.size())
            return true;

        MerchantRecipe* recipe = m_trades.at(selectedIndex).first;
        if (recipe->isDeprecated())
            return true;

        not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
        not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];

        const int matchesA = player.get()->mInventory.get()->countMatches(buyA);
        const int matchesB = player.get()->mInventory.get()->countMatches(buyB);

        if (buyA.get()->isEmpty() || matchesA < buyA.get()->getCount()
            || (!buyB.get()->isEmpty() && matchesB < buyB.get()->getCount()))
            return true;

        const int originalIndex = m_trades.at(selectedIndex).second;
        m_merchant.get()->notifyTrade(recipe);

        player.get()->mInventory.get()->removeResources(buyA);
        player.get()->mInventory.get()->removeResources(buyB);

        not_null_ptr<ItemInstance> sellItemCopy = recipe->getSellItem().get()->copy();
        if (!player.get()->mInventory.get()->add(sellItemCopy))
            player.get()->Player::drop(sellItemCopy, false);

        player.get()->mPacketListener->send(
            boost::shared_ptr<Packet>(new TradeItemPacket(m_merchantMenu->m_containerId, originalIndex)));

        updateDisplay();
        return true;
    }

    case EControllerActions_MenuLeft:
        if (m_tradeRow == 0)
        {
            if (m_tradeOffset < 1)
                return true;
            --m_tradeOffset;
        }
        else
        {
            --m_tradeRow;
            moveSelector(false);
        }
        updateDisplay();
        break;

    case EControllerActions_MenuRight:
        if (m_tradeRow == 6)
        {
            if (m_trades.empty())
                return true;
            if (static_cast<unsigned int>(m_tradeOffset + 7) >= m_trades.size())
                return true;
            ++m_tradeOffset;
        }
        else
        {
            ++m_tradeRow;
            moveSelector(true);
        }
        updateDisplay();
        break;

    default:
        return false;
    }

    selectedIndex = m_tradeRow + m_tradeOffset;
    if (m_trades.empty() || static_cast<unsigned int>(selectedIndex) >= m_trades.size())
        return true;

    const int originalIndex = m_trades.at(selectedIndex).second;
    m_merchantMenu->setSelectionHint(originalIndex);

    ByteArrayOutputStream byteStream;
    DataOutputStream dataStream(&byteStream);
    dataStream.writeInt(originalIndex);

    Minecraft::GetInstance()
        ->getConnection(m_playerId)
        ->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
            ServerboundCustomPayloadPacket::TRADER_SELECTION_PACKET, byteStream.toByteArray())));

    return true;
}

void UIScene_TradingMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (pressed)
        handled = handleKeyDown(m_playerId, action, repeat);
}

void UIScene_TradingMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!minecraft->mLocalPlayers[m_playerId])
        return;
    if (minecraft->mGameModes[m_playerId] == 0)
        return;

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slotId = -1;
    std::sscanf(name, "slot_%d", &slotId);

    if (slotId < 0x27)
    {
        Slot* slot = m_merchantMenu->getSlot(slotId);
        item = slot->getItem();
    }
    else if (slotId < 0x29)
    {
        if (!m_trades.empty() && static_cast<unsigned int>(m_tradeRow + m_tradeOffset) < m_trades.size())
        {
            item = (slotId != 0x28) ? m_trades.at(m_tradeRow + m_tradeOffset).first->getBuyAItem()
                                    : m_trades.at(m_tradeRow + m_tradeOffset).first->getBuyBItem();
        }
    }
    else if (!m_trades.empty() && static_cast<unsigned int>(slotId - 0x29 + m_tradeOffset) < m_trades.size())
    {
        item = m_trades.at(slotId - 0x29 + m_tradeOffset).first->getSellItem();
    }

    if (!item.get()->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item.get()->getItem()->isFoil(item), true, false);
}

void UIScene_TradingMenu::handleInventoryUpdated(void*)
{
    updateDisplay();
}

void UIScene_TradingMenu::HandleMessage(EUIMessage message, void* data)
{
    if (message == EUIMessage_Unknown)
        handleInventoryUpdated(data);
}

boost::shared_ptr<Merchant> UIScene_TradingMenu::getMerchant()
{
    return m_merchant;
}

int UIScene_TradingMenu::getSceneType()
{
    return EUIScene_TradingMenu;
}
