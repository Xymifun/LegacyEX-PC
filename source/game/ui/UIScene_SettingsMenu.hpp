#ifndef GAME_UI_UISCENE_SETTINGSMENU_HPP
#define GAME_UI_UISCENE_SETTINGSMENU_HPP

#include <string>

#include "platform/wiiu/C4JStorage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"

class UILayer;

class UIScene_SettingsMenu : public UIScene
{
public:
    UIScene_SettingsMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_SettingsMenu();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void tick();
    static int ResetDefaultsDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();

private:
    struct ListMenuInitData
    {
        int playerId;
        int mode;
    };

#ifdef MINECRAFT_PC
    void RemoveButtons(bool destroyControls);
#endif

    UIControl_Button m_buttons[7];
    unsigned char m_language;
};

#endif
