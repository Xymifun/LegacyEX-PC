#ifndef GAME_UI_UICOMPONENT_MENUBACKGROUND_HPP
#define GAME_UI_UICOMPONENT_MENUBACKGROUND_HPP

#include "game/client/render/C4JRender.hpp"
#include "game/ui/UIScene.hpp"

class UILayer;

class UIComponent_MenuBackground : public UIScene
{
public:
    UIComponent_MenuBackground(int sceneType, void* data, UILayer* layer);

    virtual std::wstring getMoviePath();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();

private:
    bool m_splitScreen;
};

#endif
