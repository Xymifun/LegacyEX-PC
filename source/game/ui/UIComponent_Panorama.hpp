#ifndef GAME_UI_UICOMPONENT_PANORAMA_HPP
#define GAME_UI_UICOMPONENT_PANORAMA_HPP

#include "game/ui/UIScene.hpp"

class UILayer;

class UIComponent_Panorama : public UIScene
{
public:
    UIComponent_Panorama(int sceneType, void* data, UILayer* layer);

    virtual std::wstring getMoviePath();
    virtual int getSceneType();
    virtual void tick();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void handleReload();

    void setPanorama(bool day);

private:
    bool m_splitScreen;
    bool m_panoramaDay;
    int m_currentFrame;
};

#endif
