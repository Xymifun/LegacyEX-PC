#ifndef GAME_UI_UISCENE_SETTINGSLISTMENU_HPP
#define GAME_UI_UISCENE_SETTINGSLISTMENU_HPP

#include <stdint.h>
#include <string>

#include "game/core/GameSettings.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIScene.hpp"

class UIScene_SettingsListMenu : public UIScene
{
public:
    UIScene_SettingsListMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_SettingsListMenu();

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePreUnloadForReload();
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);

    void doVerticalResizeCheck();
    void HideDescription();
    void Init();
    void setGameSettings();

private:
    void addSettingCheckbox(int playerId, const wchar_t* label, int id, eGameSetting setting);
    void addInvertedSettingCheckbox(int playerId, const wchar_t* label, int id, eGameSetting setting);

    UIControl_MultiList m_multiList;
    UIControl_HTMLLabel m_description;
    unsigned int m_settingsMode;
    bool m_gameStarted;
    bool m_primaryPlayer;
    bool m_host;
    bool m_saveRequested;
    wchar_t m_skinNames[9][256];
    wchar_t m_difficultyNames[4][256];
};

#endif
