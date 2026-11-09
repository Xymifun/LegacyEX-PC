#ifndef GAME_UI_UISCENE_MESSAGEBOX_HPP
#define GAME_UI_UISCENE_MESSAGEBOX_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/core/CMinecraftApp.hpp"

class UIScene_MessageBox : public UIScene
{
public:
    UIScene_MessageBox(int sceneType, void* sceneData, UILayer* layer);
    virtual ~UIScene_MessageBox();

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void updateTooltips();
    virtual void handleReload();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual bool hasFocus(int sceneType);
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();
    virtual unsigned int getSubSceneType() const;
    virtual bool hidesLowerScenes();
    virtual bool blocksInput();

private:
    int (*m_callback)(void*, int, C4JStorage::EMessageResult);
    void* m_callbackData;
    int m_titleId;
    int m_buttonCount;
    bool m_alert;
#ifdef MINECRAFT_PC
    bool m_wide;
#endif
    UIControl_Button m_buttons[4];
    UIControl_Label m_title;
    UIControl_Label m_content;
};

#endif
