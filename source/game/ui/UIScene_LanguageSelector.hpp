#ifndef GAME_UI_UISCENE_LANGUAGESELECTOR_HPP
#define GAME_UI_UISCENE_LANGUAGESELECTOR_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_DynamicButtonList.hpp"

class UILayer;

class UIScene_LanguageSelector : public UIScene
{
public:
    UIScene_LanguageSelector(int sceneType, void* data, UILayer* layer);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);

private:
    UIControl_DynamicButtonList m_languageList;
};

#endif
