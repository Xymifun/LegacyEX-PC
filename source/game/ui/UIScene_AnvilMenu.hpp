#ifndef GAME_UI_UISCENE_ANVILMENU_HPP
#define GAME_UI_UISCENE_ANVILMENU_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_TextInput.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/core/TickAction.hpp"
#include "game/ui/MethodAction.hpp"

class AbstractContainerMenu;
class AnvilMenu;
class CensoredString;
class Container;
class Inventory;
class ItemInstance;
class UILayer;
class UIScene_AnvilMenu : public UIScene_AbstractContainerMenu, public net_minecraft_world_inventory::ContainerListener
{
public:
    struct PendingNameInfo
    {
        std::wstring name;
        std::wstring itemName;
    };

    UIScene_AnvilMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_AnvilMenu();

    virtual int getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position);
    virtual void GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position,
                                   _UIVec2D* size);
    virtual void handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction,
                                        bool wrap);
    virtual unsigned int GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                      IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y);
    virtual int getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void handleOtherClicked(int button, IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                    bool pressed);
    virtual bool IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section);

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual void handleReload();
    virtual void tick();
    virtual void handleDestroy();
    virtual int getSceneType();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);

    void addPendingNameChange(const PendingNameInfo& info);
    static int KeyboardCompleteCallback(void* data, bool accepted);
    void handleEditNamePressed();
    virtual void refreshContainer(AbstractContainerMenu* menu, std::vector<not_null_ptr<ItemInstance> >* items);
    virtual void slotChanged(AbstractContainerMenu* menu, int slot, not_null_ptr<ItemInstance> item);
    virtual void setContainerData(AbstractContainerMenu* menu, int id, int value);
    virtual void setAllContainerData(AbstractContainerMenu* menu, boost::shared_ptr<Container> container);
    void setCostLabel(const std::wstring& label, bool safe);
    void showCross(bool show);
    void handleTick();
    void setEditNameValue(const std::wstring& value);
    void updateItemName();
    void setEditNameEditable(bool editable);

private:
    PendingNameInfo* m_pendingName;
    boost::shared_ptr<Inventory> m_inventory;
    AnvilMenu* m_anvilMenu;
    boost::shared_ptr<CensoredString> m_itemName;
    MethodAction<UIScene_AnvilMenu> m_nameUpdateAction;
    bool m_crossVisible;
    UIControl_SlotList m_ingredientSlot;
    UIControl_SlotList m_ingredientSlot2;
    UIControl_SlotList m_resultSlot;
    UIControl_Label m_anvilLabel;
    UIControl_TextInput m_editName;
};

#endif
