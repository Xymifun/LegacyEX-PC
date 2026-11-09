#ifndef GAME_UI_UICOMPONENT_CHAT_HPP
#define GAME_UI_UICOMPONENT_CHAT_HPP

#include "game/client/render/C4JRender.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIScene.hpp"

class UILayer;

class UIComponent_Chat : public UIScene
{
public:
    UIComponent_Chat(int type, void* data, UILayer* layer);

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void handleTimerComplete(int timerId);
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();

private:
    bool m_splitScreen;
    UIControl_Label m_labels[10];
    UIControl_Label m_chatLabel;
    UIControl m_backgrounds[10];
};

#endif
