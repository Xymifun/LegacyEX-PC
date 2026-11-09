#ifndef GAME_UI_UISCENE_DLCOFFERSMENU_HPP
#define GAME_UI_UISCENE_DLCOFFERSMENU_HPP

#include <stdint.h>
#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_DLCButtonList.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UILayer;
class cProductInfo;

class UIScene_DLCOffersMenu : public UIScene
{
public:
    UIScene_DLCOffersMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_DLCOffersMenu();

    virtual unsigned int mapElementsAndNames();
    static uint32_t ExitDLCOffersMenu(void* context, int pad, C4JStorage::EMessageResult result);
    uint32_t UpdateDisplay(cProductInfo* product);
    virtual void tick();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleSelectionChanged(int selection);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleTimerComplete(int timerId);
    virtual void HandleDLCLicenseChange();
    virtual int getSceneType();

private:
    bool m_isStandardDefinition;
    bool m_alreadyInstalled;
    bool m_installable;
    bool m_navigateBack;
    UIControl_DLCButtonList m_offerList;
    UIControl_Label m_titleLabel;
    UIControl_Label m_priceLabel;
    UIControl_HTMLLabel m_descriptionLabel;
    UIControl_BitmapIcon m_banner;
    UIControl m_timer;
    int m_unknown364;
    int m_unknown368;
    bool m_waitingForCatalog;
    bool m_displayUpdated;
    int m_category;
    int m_selection;
    unsigned int m_productCount;
    bool m_populatePending;
    cProductInfo* m_products;
};

#endif
