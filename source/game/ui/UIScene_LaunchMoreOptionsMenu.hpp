#ifndef GAME_UI_UISCENE_LAUNCHMOREOPTIONSMENU_HPP
#define GAME_UI_UISCENE_LAUNCHMOREOPTIONSMENU_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/core/SliderLogic.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIControl_Touch.hpp"

class LaunchMoreOptionsMenuInitData;
class UILayer;

class UIScene_LaunchMoreOptionsMenu : public UIScene
{
public:
    UIScene_LaunchMoreOptionsMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_LaunchMoreOptionsMenu();

    void EnableCustomOptions(bool enabled);
    void UpdateCheats();
    void UpdateDescriptionText();
    void _init();
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    static int KeyboardCompleteSeedCallback(void* data, bool accepted);
    void HandleRequestKeyboard();
    void UpdateOnline();
    virtual void tick();
    virtual void handleDestroy();
    virtual void handleReload();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleTimerComplete(int timerId);
    virtual void handlePress(int controlId, int childId);
    virtual void handleSliderElementMove(int first, int second, int value);
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual UIControl* GetMainPanel();
    virtual int getSceneType();

private:
    int m_focusedControlId;
    int m_focusedItemId;
    SliderLogic m_worldSize;
    SliderLogic m_worldType;
    SliderLogic m_gameMode;
    SliderLogic m_lootSet;
    SliderLogic m_foodBalance;
    SliderLogic m_mapSize;
    SliderLogic m_roundLength;
    UIControl_Touch m_worldTab;
    UIControl_Touch m_gameTab;
    UIControl m_unmappedControlA;
    UIControl m_unmappedControlB;
    UIControl m_worldOptions;
    UIControl m_gameOptions;
    UIControl_MultiList m_worldOptionsList;
    UIControl_MultiList m_gameOptionsList;
    UIControl_Label m_worldLabel;
    UIControl_Label m_gameLabel;
    UIControl_Label m_unmappedLabelA;
    UIControl_HTMLLabel m_gameDescription;
    UIControl_HTMLLabel m_worldDescription;
    UIControl_Label m_unmappedLabelB;
    UIControl_Label m_unmappedLabelC;
    UIControl_Label m_worldSizeLabel;
    UIControl_Label m_resizeLabel;
    LaunchMoreOptionsMenuInitData* m_initData;
    bool m_online;
    bool m_keyboardActive;
    int m_menuType;
    bool m_requestKeyboard;
    bool m_updateCheats;
    bool m_updateOnline;
    std::wstring m_seed;
};

#endif
