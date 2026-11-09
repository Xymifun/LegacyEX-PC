#ifndef GAME_UI_UISCENE_CONTAINERMENU_HPP
#define GAME_UI_UISCENE_CONTAINERMENU_HPP

#include <string>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/ui/UIControl_Label.hpp"

class UILayer;

class UIScene_ContainerMenu : public UIScene_AbstractContainerMenu
{
public:
    UIScene_ContainerMenu(int playerId, void* data, UILayer* layer);
    virtual ~UIScene_ContainerMenu();

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
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSceneType();

private:
    bool m_largeChest;
    UIControl_SlotList m_containerList;
    UIControl_Label m_chestLabel;
};

#endif
