#ifndef GAME_UI_UISCENE_HORSEINVENTORYMENU_HPP
#define GAME_UI_UISCENE_HORSEINVENTORYMENU_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MinecraftHorse.hpp"
#include "game/ui/UIControl_SlotList.hpp"

class AbstractChestedHorse;
class AbstractHorse;
class Container;
class UILayer;

class UIScene_HorseInventoryMenu : public UIScene_AbstractContainerMenu, public virtual IUIScene_AbstractContainerMenu
{
    friend class UIControl_MinecraftHorse;

public:
    UIScene_HorseInventoryMenu(int player, void* data, UILayer* layer);

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
    virtual bool IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int IsVisible(IUIScene_AbstractContainerMenu::ESceneSection section);

    void SetInventoryType(int type);
    void SetHasInventory(bool hasInventory);
    void SetHasSaddle(bool hasSaddle);
    void setUp(bool enabled);

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual int getSceneType();

private:
    boost::shared_ptr<Container> m_playerInventory;
    boost::shared_ptr<Container> m_container;
    boost::shared_ptr<AbstractHorse> m_horse;
    UIControl_SlotList m_saddleSlot;
    UIControl_SlotList m_armorSlot;
    UIControl_SlotList m_donkeyInventoryList;
    UIControl_Label m_horseLabel;
    UIControl_MinecraftHorse m_horseControl;
};

#endif
