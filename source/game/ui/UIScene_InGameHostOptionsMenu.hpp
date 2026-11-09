#ifndef GAME_UI_UISCENE_INGAMEHOSTOPTIONSMENU_HPP
#define GAME_UI_UISCENE_INGAMEHOSTOPTIONSMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_MultiList.hpp"

class UIScene_InGameHostOptionsMenu : public UIScene
{
public:
    UIScene_InGameHostOptionsMenu(int sceneType, void* data, UILayer* layer);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void updateTooltips();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);

    unsigned int getWeatherId(int weatherState);
    void Init();

private:
    UIControl_MultiList m_hostOptions;
};

#endif
