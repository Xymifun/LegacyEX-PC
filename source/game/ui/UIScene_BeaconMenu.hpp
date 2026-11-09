#ifndef GAME_UI_UISCENE_BEACONMENU_HPP
#define GAME_UI_UISCENE_BEACONMENU_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_BeaconEffectButton.hpp"
#include "game/ui/UIControl_SlotList.hpp"

class UILayer;
class Container;
class MobEffect;
class UIScene_BeaconMenu : public UIScene_AbstractContainerMenu
{
public:
    UIScene_BeaconMenu(int type, void* data, UILayer* layer);

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleReload();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int GetBaseSlotCount();
    virtual unsigned int GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                      IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y);
    virtual int getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void handleOtherClicked(int button, IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                    bool pressed);
    virtual bool IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual std::vector<HtmlString>* GetSectionHoverText(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int IsVisible(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position);
    virtual void GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position,
                                   _UIVec2D* size);
    virtual void handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction,
                                        bool wrap);

    int GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection section);
    unsigned int GetPowerButtonTier(IUIScene_AbstractContainerMenu::ESceneSection section);
    MobEffect* GetPowerButtonEffect(IUIScene_AbstractContainerMenu::ESceneSection section);
    unsigned char IsPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section);
    void SetPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section);
    void AddPowerButton(int id, int icon, int tier, int column, bool active, bool selected);
    void SetConfirmButtonEnabled(bool enabled);
    void handleTick();

private:
    UIControl_SlotList m_activatorSlot;
    UIControl_SlotList m_activatorList;
    UIControl_Label m_primaryPowerLabel;
    UIControl_Label m_secondaryPowerLabel;
    UIControl_BeaconEffectButton m_powerButtons[7];
    UIControl_BeaconEffectButton m_confirmButton;
    boost::shared_ptr<Container> m_beaconContainer;
    int m_primaryEffect;
    int m_secondaryEffect;
    int m_beaconLevel;
    int m_nextButtonId;
};

#endif
