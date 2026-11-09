#ifndef GAME_UI_UICOMPONENT_DEBUGUICONSOLE_HPP
#define GAME_UI_UICOMPONENT_DEBUGUICONSOLE_HPP

#include <deque>
#include <string>

#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIScene.hpp"

class UIComponent_DebugUIConsole : public UIScene
{
public:
    UIComponent_DebugUIConsole(int sceneType, void* data, UILayer* layer);

    virtual unsigned int mapElementsAndNames();
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();

    void addText(const std::string& text);

private:
    UIControl_Label m_consoleLines[10];
    std::deque<std::string> m_text;
    bool m_updateLabels;
};

#endif
