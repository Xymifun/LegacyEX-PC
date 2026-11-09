#ifndef GAME_UI_UISCENE_INVENTORYMENU_HPP
#define GAME_UI_UISCENE_INVENTORYMENU_HPP

#include <string>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MinecraftPlayer.hpp"
#include "game/ui/UIControl_SlotList.hpp"

class UILayer;

class UIScene_InventoryMenu : public UIScene_AbstractContainerMenu, public virtual IUIScene_AbstractContainerMenu
{
public:
    UIScene_InventoryMenu(int pad, void* data, UILayer* layer);
    virtual ~UIScene_InventoryMenu();
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
    void SetClassicCrafting(bool classicCrafting);
    void updateEffectsDisplay();
    virtual unsigned int getSubSceneType() const;
    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleTimerComplete(int timerId);
    virtual int getSceneType();

private:
    int* m_effectDurations;
    UIControl_SlotList m_armorList;
    UIControl_SlotList m_craftingList;
    UIControl_SlotList m_resultList;
    UIControl_SlotList m_shieldList;
    UIControl_MinecraftPlayer m_playerControl;
    UIControl_Label m_craftingLabel;
    bool m_classicCrafting;
};

#endif
