#ifndef GAME_UI_UISCENE_ABSTRACTCONTAINERMENU_HPP
#define GAME_UI_UISCENE_ABSTRACTCONTAINERMENU_HPP

#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Cursor.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/core/Slot.hpp"

class AbstractContainerMenu;
class HtmlString;
class ItemInstance;
class Slot;
class UILayer;

class UIScene_AbstractContainerMenu : public UIScene, public virtual IUIScene_AbstractContainerMenu
{
public:
    UIScene_AbstractContainerMenu(int type, UILayer* layer);
    virtual ~UIScene_AbstractContainerMenu();

    virtual int getPad();
    virtual void PlatformInitialize(bool navigateBack, int pad, int startIndex);
    virtual void InitDataAssociations(int pad, AbstractContainerMenu* menu, int startIndex);
    virtual bool doesSectionTreeHaveFocus(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void setSectionFocus(IUIScene_AbstractContainerMenu::ESceneSection section, int pad);
    virtual void setFocusToPointer(int pad);
    virtual void SetPointerText(std::vector<HtmlString>* description, bool newSlot);
    virtual not_null_ptr<ItemInstance> getSlotItem(IUIScene_AbstractContainerMenu::ESceneSection section, int slot);
    virtual Slot* getSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot);
    virtual bool isSlotEmpty(IUIScene_AbstractContainerMenu::ESceneSection section, int slot);
    virtual void adjustPointerForSafeZone();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int GetBaseSlotCount();

    virtual unsigned int mapElementsAndNames();
    virtual void handleDestroy();
    virtual void tick();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void updateTooltips();

protected:
    IUIScene_AbstractContainerMenu::ESceneSection m_focusSection;
    bool m_ignoreInput;
    int m_movieWidth;
    int m_movieHeight;
    UIControl m_mainPanel;
    UIControl_SlotList m_hotbarList;
    UIControl_SlotList m_inventoryList;
    UIControl_Cursor m_cursor;
    UIControl_Label m_inventoryLabel;
    UIControl_Label m_hotbarLabel;
    UIControl m_backgroundPanel;
};

#endif
