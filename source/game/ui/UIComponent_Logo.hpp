#ifndef GAME_UI_UICOMPONENT_LOGO_HPP
#define GAME_UI_UICOMPONENT_LOGO_HPP
#include <string>
#include "game/ui/UIScene.hpp"
class UILayer;
class FJ_ComponentLogo;
class UIComponent_Logo : public UIScene
{
public:
    UIComponent_Logo(int sceneType, void* callback, UILayer* layer);
    void setLogoLanguage(int language);
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int controller);
    virtual bool hidesLowerScenes();
};
#endif
