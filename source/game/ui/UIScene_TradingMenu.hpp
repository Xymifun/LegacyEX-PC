#ifndef GAME_UI_UISCENE_TRADINGMENU_HPP
#define GAME_UI_UISCENE_TRADINGMENU_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/Tutorial.hpp"

class Merchant;
class MerchantMenu;
class MerchantRecipe;
class UILayer;
struct fuiRect;
class ItemInstance;
class HtmlString;

class UIScene_TradingMenu : public UIScene
{
public:
    UIScene_TradingMenu(int iPad, void* initData, UILayer* layer);
    virtual ~UIScene_TradingMenu();

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void updateTooltips();
    virtual void handleDestroy();
    std::vector<HtmlString>* GetItemDescription(not_null_ptr<ItemInstance> item);
    bool canMake(MerchantRecipe* recipe);
    void setTradeRedBox(int trade, bool show);
    void setTitle(const std::wstring& title);
    void setOfferDescription(std::vector<HtmlString>* description);
    void setRequest1Name(const std::wstring& name);
    void setRequest2Name(const std::wstring& name);
    void setRequest1RedBox(bool show);
    void setRequest2RedBox(bool show);
    void updateDisplay();
    virtual void handleReload();
    void showScrollRightArrow(bool show);
    void showScrollLeftArrow(bool show);
    void handleTick();
    virtual void tick();
    void moveSelector(bool right);
    bool handleKeyDown(int iPad, int action, bool repeat);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void customDraw(const char* name, fuiRect* rect);
    void handleInventoryUpdated(void* data);
    virtual void HandleMessage(EUIMessage message, void* data);
    boost::shared_ptr<Merchant> getMerchant();
    virtual int getSceneType();

private:
    bool m_scrollRightShown;
    bool m_scrollLeftShown;
    MerchantMenu* m_merchantMenu;
    boost::shared_ptr<Merchant> m_merchant;
    std::vector<std::pair<MerchantRecipe*, int> > m_trades;
    int m_tradeCount;
    int m_tradeRow;
    int m_tradeOffset;
    bool m_tradeFocusSet;
    eTutorial_State m_previousTutorialState;
    UIControl m_mainPanel;
    UIControl_SlotList m_tradingBar;
    UIControl_SlotList m_request1;
    UIControl_SlotList m_request2;
    UIControl_SlotList m_hotBar;
    UIControl_SlotList m_inventory;
    UIControl_Label m_inventoryLabel;
    UIControl_Label m_villagerText;
    UIControl_Label m_requiredLabel;
    UIControl_Label m_request1Label;
    UIControl_Label m_request2Label;
};

#endif
