#ifndef GAME_UI_UISCENE_INGAMESETTINGSLISTMENU_HPP
#define GAME_UI_UISCENE_INGAMESETTINGSLISTMENU_HPP

#include "game/core/MultiListSliderLogic.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"

class UIScene_InGameSettingsListMenu : public UIScene
{
public:
    UIScene_InGameSettingsListMenu(int sceneType, void* data, UILayer* layer);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual ~UIScene_InGameSettingsListMenu();
    virtual int getSceneType();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);

    void EnableCustomOptions(bool enabled);
    void doVerticalResizeCheck();
    void Init();
    void UpdateSettings(bool unavailable);
    void HideDescription();
    void setGameSettings();
    void UpdateDescriptionText();

private:
    double m_focus;
    double m_childFocus;
    MultiListSliderLogic m_roundLengthSlider;
    MultiListSliderLogic m_maxPlayersSlider;
    MultiListSliderLogic m_gameVariantSlider;
    UIControl_MultiList m_multiList;
    UIControl_HTMLLabel m_description;
    LaunchMoreOptionsMenuInitData m_launchData;
    MiniGamePlayerSettings m_playerSettings;
    MiniGamePlayerSettings m_originalPlayerSettings;
    int m_mapSelection;
    EMiniGameId m_miniGameId;
    bool m_customGame;
    bool m_gameStarted;
    bool m_primaryPlayer;
    bool m_host;
};

#endif
