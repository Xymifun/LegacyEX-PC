#ifndef GAME_UI_UICOMPONENT_DEBUGUIMARKETINGGUIDE_HPP
#define GAME_UI_UICOMPONENT_DEBUGUIMARKETINGGUIDE_HPP

#include <string>
#include "game/ui/UIScene.hpp"

class UIComponent_DebugUIMarketingGuide : public UIScene
{
public:
    UIComponent_DebugUIMarketingGuide(int type, void* data, UILayer* layer);
    virtual std::wstring getMoviePath();
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int controllerId);
    virtual bool hidesLowerScenes();
};

#endif
