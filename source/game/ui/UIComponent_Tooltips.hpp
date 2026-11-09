#ifndef GAME_UI_UICOMPONENT_TOOLTIPS_HPP
#define GAME_UI_UICOMPONENT_TOOLTIPS_HPP

#include <string>

#include "game/core/GameSettings.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIControl_Touch.hpp"

class UILayer;

class UIComponent_Tooltips : public UIScene
{
public:
    class _TooltipValues
    {
    public:
        bool m_enabled;
        int m_textId;
        UIString m_text;
        int m_type;
        int m_subtype;
        bool m_dynamic;
        EControllerActions m_button;

        _TooltipValues();
    };

    UIComponent_Tooltips(int sceneType, void* data, UILayer* layer);
    virtual ~UIComponent_Tooltips() {}

    void overrideSFX(int pad, int action, bool enabled);
    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual double getSafeZoneHalfWidth();
    virtual void updateSafeZone();
    void _SetTooltip(unsigned int index, int type, int subtype, UIString text, bool enabled, bool force);
    void _Relayout();
    virtual void tick();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void updateViewportTouchOffset();
    unsigned int _SetTooltip(unsigned int index, int value);
    virtual void SetTooltipText(unsigned int player, int index);
    virtual void SetEnableTooltips(bool enabled);
    virtual void ShowTooltip(unsigned int player, bool visible);
    unsigned int GetToolTipButton(unsigned int button);
    unsigned int _SetTooltip(unsigned int index, int type, int subtype, int textId, bool& changed);
    unsigned int HandleToolTipButton(unsigned int player, EControllerActions action, unsigned int& button, int type);
    void UpdateControllerType(int controllerType);
    virtual void SetTooltips(int player, int* tooltips, bool visible);
    virtual void SetTooltips(int player, int first, int second, int third, int fourth, int fifth, int sixth,
                             int seventh, int eighth, int ninth, int tenth, int eleventh, int twelfth, int thirteenth,
                             int fourteenth, int fifteenth, int sixteenth, bool visible);
    virtual void EnableTooltip(unsigned int player, bool enabled);
    virtual bool IsToolTipDynamic(unsigned int player, EControllerActions& action);
    virtual void handleReload();
    virtual void handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();

private:
    bool m_splitTooltips;
    _TooltipValues m_tooltipValues[16];
    unsigned int m_controllerType;
    UIControl_Touch m_touchControls[16];
    bool m_overrideSFX[4][23];
};

#endif
