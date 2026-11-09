#ifndef GAME_UI_UISCENE_HOWTOPLAYMENU_HPP
#define GAME_UI_UISCENE_HOWTOPLAYMENU_HPP

#include "game/ui/UIControl_ButtonList.hpp"
#include "game/ui/UIScene.hpp"

class UIScene_HowToPlayMenu : public UIScene
{
public:
    UIScene_HowToPlayMenu(int sceneType, void* sceneData, UILayer* layer);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);

private:
    static unsigned int m_buttonLabels[33];

    UIControl_ButtonList m_buttonList;
    int m_selectedItem;
};

#endif
