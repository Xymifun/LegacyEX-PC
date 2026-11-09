#ifndef PLATFORM_WIIU_UICOMPONENT_WIIUDRC_HPP
#define PLATFORM_WIIU_UICOMPONENT_WIIUDRC_HPP
#include <string>
#include "game/ui/UIScene.hpp"
class UILayer;
class ConsoleUIController;
class UIComponent_WiiUDRC : public UIScene
{
private:
    friend class ConsoleUIController;
    int m_iconX;
    int m_iconY;
    int m_iconWidth;
    int m_iconHeight;
    bool m_drcOutputEnabled;
    bool m_drcIconEnabled;
    unsigned int m_touchX;
    unsigned int m_touchY;
    bool m_touchingIcon;
    bool m_touchPressed;

public:
    UIComponent_WiiUDRC(int sceneType, void* param, UILayer* layer);
    int GetIconX();
    int GetIconWidth();
    int GetIconY();
    int GetIconHeight();
    void handleDRCTouchInput(unsigned int playerIndex);
    virtual std::wstring getMoviePath();
    void SetEnableBackground(bool enable);
    void SetDRCIcon(bool enable, bool connected);
    virtual void tick();
    virtual void handleReload();
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int playerIndex);
    virtual bool hidesLowerScenes();
};
#endif
