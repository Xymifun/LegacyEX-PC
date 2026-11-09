#ifndef GAME_UI_UISCENE_FURNACEMENU_HPP
#define GAME_UI_UISCENE_FURNACEMENU_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Progress.hpp"
#include "game/ui/UIControl_SlotList.hpp"

class Container;
class FurnaceMenu;
class Inventory;
class UILayer;

class UIScene_FurnaceMenu : public UIScene_AbstractContainerMenu
{
public:
    UIScene_FurnaceMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_FurnaceMenu();

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
    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual void tick();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSceneType();

private:
    boost::shared_ptr<Container> m_container;
    UIControl_SlotList m_fuelSlot;
    UIControl_SlotList m_ingredientSlot;
    UIControl_SlotList m_resultSlot;
    UIControl_Label m_furnaceLabel;
    UIControl_Label m_ingredientLabel;
    UIControl_Label m_fuelLabel;
    UIControl_Progress m_furnaceFire;
    UIControl_Progress m_furnaceArrow;
};

#endif
